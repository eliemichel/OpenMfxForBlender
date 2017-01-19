/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) Blender Foundation
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/blenkernel/intern/collision.c
 *  \ingroup bke
 */


#include "MEM_guardedalloc.h"

#include "DNA_cloth_types.h"
#include "DNA_effect_types.h"
#include "DNA_group_types.h"
#include "DNA_object_types.h"
#include "DNA_object_force.h"
#include "DNA_scene_types.h"
#include "DNA_meshdata_types.h"

#include "BLI_utildefines.h"
#include "BLI_blenlib.h"
#include "BLI_math.h"
#include "BLI_edgehash.h"

#include "BKE_cloth.h"
#include "BKE_effect.h"
#include "BKE_modifier.h"
#include "BKE_scene.h"

#ifdef WITH_BULLET
#include "Bullet-C-Api.h"
#endif
#include "BLI_kdopbvh.h"
#include "BKE_collision.h"

#ifdef WITH_ELTOPO
#include "eltopo-capi.h"
#endif


/***********************************
Collision modifier code start
***********************************/

/* step is limited from 0 (frame start position) to 1 (frame end position) */
void collision_move_object(CollisionModifierData *collmd, float step, float prevstep)
{
	float tv[3] = {0, 0, 0};
	unsigned int i = 0;

	/* the collider doesn't move this frame */
	if (collmd->is_static) {
		for (i = 0; i < collmd->mvert_num; i++) {
			zero_v3(collmd->current_v[i].co);
		}

		return;
	}

	for (i = 0; i < collmd->mvert_num; i++) {
		sub_v3_v3v3(tv, collmd->xnew[i].co, collmd->x[i].co);
		VECADDS(collmd->current_x[i].co, collmd->x[i].co, tv, prevstep);
		VECADDS(collmd->current_xnew[i].co, collmd->x[i].co, tv, step);
		sub_v3_v3v3(collmd->current_v[i].co, collmd->current_xnew[i].co, collmd->current_x[i].co);
	}

	bvhtree_update_from_mvert(
	        collmd->bvhtree, collmd->current_x, collmd->current_xnew,
	        collmd->tri, collmd->tri_num, true);
}

BVHTree *bvhtree_build_from_mvert(
        const MVert *mvert,
        const struct MVertTri *tri, int tri_num,
        float epsilon)
{
	BVHTree *tree;
	const MVertTri *vt;
	int i;

	tree = BLI_bvhtree_new(tri_num, epsilon, 4, 26);

	/* fill tree */
	for (i = 0, vt = tri; i < tri_num; i++, vt++) {
		float co[3][3];

		copy_v3_v3(co[0], mvert[vt->tri[0]].co);
		copy_v3_v3(co[1], mvert[vt->tri[1]].co);
		copy_v3_v3(co[2], mvert[vt->tri[2]].co);

		BLI_bvhtree_insert(tree, i, co[0], 3);
	}

	/* balance tree */
	BLI_bvhtree_balance(tree);

	return tree;
}

void bvhtree_update_from_mvert(
        BVHTree *bvhtree,
        const MVert *mvert, const MVert *mvert_moving,
        const MVertTri *tri, int tri_num,
        bool moving)
{
	const MVertTri *vt;
	int i;

	if ((bvhtree == NULL) || (mvert == NULL)) {
		return;
	}

	if (mvert_moving == NULL) {
		moving = false;
	}

	for (i = 0, vt = tri; i < tri_num; i++, vt++) {
		float co[3][3];
		bool ret;

		copy_v3_v3(co[0], mvert[vt->tri[0]].co);
		copy_v3_v3(co[1], mvert[vt->tri[1]].co);
		copy_v3_v3(co[2], mvert[vt->tri[2]].co);

		/* copy new locations into array */
		if (moving) {
			float co_moving[3][3];
			/* update moving positions */
			copy_v3_v3(co_moving[0], mvert_moving[vt->tri[0]].co);
			copy_v3_v3(co_moving[1], mvert_moving[vt->tri[1]].co);
			copy_v3_v3(co_moving[2], mvert_moving[vt->tri[2]].co);

			ret = BLI_bvhtree_update_node(bvhtree, i, &co[0][0], &co_moving[0][0], 3);
		}
		else {
			ret = BLI_bvhtree_update_node(bvhtree, i, &co[0][0], NULL, 3);
		}

		/* check if tree is already full */
		if (ret == false) {
			break;
		}
	}

	BLI_bvhtree_update_tree(bvhtree);
}

/***********************************
Collision modifier code end
***********************************/

// w3 is not perfect
static void collision_compute_barycentric ( float pv[3], float p1[3], float p2[3], float p3[3], float *w1, float *w2, float *w3 )
{
	/* dot_v3v3 */
#define INPR(v1, v2) ( (v1)[0] * (v2)[0] + (v1)[1] * (v2)[1] + (v1)[2] * (v2)[2])

	double	tempV1[3], tempV2[3], tempV4[3];
	double	a, b, c, d, e, f;

	VECSUB ( tempV1, p1, p3 );
	VECSUB ( tempV2, p2, p3 );
	VECSUB ( tempV4, pv, p3 );

	a = INPR ( tempV1, tempV1 );
	b = INPR ( tempV1, tempV2 );
	c = INPR ( tempV2, tempV2 );
	e = INPR ( tempV1, tempV4 );
	f = INPR ( tempV2, tempV4 );

	d = ( a * c - b * b );

	if ( ABS ( d ) < (double)ALMOST_ZERO ) {
		*w1 = *w2 = *w3 = 1.0 / 3.0;
		return;
	}

	w1[0] = ( float ) ( ( e * c - b * f ) / d );

	if ( w1[0] < 0 )
		w1[0] = 0;

	w2[0] = ( float ) ( ( f - b * ( double ) w1[0] ) / c );

	if ( w2[0] < 0 )
		w2[0] = 0;

	w3[0] = 1.0f - w1[0] - w2[0];

#undef INPR
}

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif

DO_INLINE void collision_interpolateOnTriangle ( float to[3], float v1[3], float v2[3], float v3[3], double w1, double w2, double w3 )
{
	zero_v3(to);
	VECADDMUL(to, v1, w1);
	VECADDMUL(to, v2, w2);
	VECADDMUL(to, v3, w3);
}

static int cloth_collision_response_static (ClothModifierData *clmd, CollisionModifierData *collmd,
                                            CollPair *collpair, CollPair *collision_end, float friction)
{
	int result = 0;
	Cloth *cloth1;
	float w1, w2, w3, u1, u2, u3;
	float v1[3], v2[3], relativeVelocity[3];
	float magrelVel;
	float epsilon2 = BLI_bvhtree_get_epsilon ( collmd->bvhtree );
	float collider_norm[3];
	bool backside;

	cloth1 = clmd->clothObject;

	for ( ; collpair != collision_end; collpair++ ) {
		float i1[3], i2[3], i3[3];

		zero_v3(i1);
		zero_v3(i2);
		zero_v3(i3);

		/* only handle static collisions here */
		if ( collpair->flag & COLLISION_IN_FUTURE )
			continue;

		/* compute barycentric coordinates for both collision points */
		collision_compute_barycentric ( collpair->pa,
			cloth1->verts[collpair->ap1].txold,
			cloth1->verts[collpair->ap2].txold,
			cloth1->verts[collpair->ap3].txold,
			&w1, &w2, &w3 );

		/* was: txold */
		collision_compute_barycentric ( collpair->pb,
			collmd->current_x[collpair->bp1].co,
			collmd->current_x[collpair->bp2].co,
			collmd->current_x[collpair->bp3].co,
			&u1, &u2, &u3 );

		/* compute collision normal */
		if (clmd->coll_parms->flags & CLOTH_COLLSETTINGS_FLAG_USE_NORMAL) {
			normal_tri_v3(collider_norm, collmd->current_x[collpair->bp1].co, collmd->current_x[collpair->bp2].co, collmd->current_x[collpair->bp3].co);
			backside = dot_v3v3(collider_norm, collpair->normal) < 0.0f;
		}
		else {
			copy_v3_v3(collider_norm, collpair->normal);
			backside = false;
		}

		/* Calculate relative "velocity". */
		collision_interpolateOnTriangle ( v1, cloth1->verts[collpair->ap1].tv, cloth1->verts[collpair->ap2].tv, cloth1->verts[collpair->ap3].tv, w1, w2, w3 );

		collision_interpolateOnTriangle ( v2, collmd->current_v[collpair->bp1].co, collmd->current_v[collpair->bp2].co, collmd->current_v[collpair->bp3].co, u1, u2, u3 );

		sub_v3_v3v3(relativeVelocity, v2, v1);

		/* Calculate the normal component of the relative velocity (actually only the magnitude - the direction is stored in 'normal'). */
		magrelVel = dot_v3v3(relativeVelocity, collider_norm);

		/* printf("magrelVel: %f\n", magrelVel); */

		/* Calculate masses of points.
		 * TODO */

		/* If v_n_mag < 0 the edges are approaching each other. */
		if ( magrelVel > ALMOST_ZERO ) {
			/* Calculate Impulse magnitude to stop all motion in normal direction. */
			float magtangent = 0, repulse = 0, d = 0;
			double impulse = 0.0;
			float vrel_t_pre[3];
			float temp[3], spf;

			/* calculate tangential velocity */
			copy_v3_v3 ( temp, collider_norm );
			mul_v3_fl(temp, magrelVel);
			sub_v3_v3v3(vrel_t_pre, relativeVelocity, temp);

			/* Decrease in magnitude of relative tangential velocity due to coulomb friction
			 * in original formula "magrelVel" should be the "change of relative velocity in normal direction" */
			magtangent = min_ff(friction * 0.01f * magrelVel, len_v3(vrel_t_pre));

			/* Apply friction impulse. */
			if ( magtangent > ALMOST_ZERO ) {
				normalize_v3(vrel_t_pre);

				/*impulse = magtangent / ( 1.0f + w1*w1 + w2*w2 + w3*w3 );  2.0 * */

				/* Impulse should be uniform throughout polygon, the scaling used above was wrong */
				impulse = magtangent / 1.5;

				VECADDMUL ( i1, vrel_t_pre, w1 * impulse );
				VECADDMUL ( i2, vrel_t_pre, w2 * impulse );
				VECADDMUL ( i3, vrel_t_pre, w3 * impulse );
			}

			/* Apply velocity stopping impulse
			 * I_c = m * v_N / 2.0
			 * no 2.0 * magrelVel normally, but looks nicer DG */
			/*impulse =  magrelVel / ( 1.0 + w1*w1 + w2*w2 + w3*w3 );*/

			/* Impulse should be uniform throughout polygon, the scaling used above was wrong */
			impulse =  magrelVel / 1.5f;

			if (backside) {
				float pt_relvel[3];
				float pt_magvel;

				sub_v3_v3v3(pt_relvel, collmd->current_v[collpair->bp1].co, cloth1->verts[collpair->ap1].tv);
				pt_magvel = dot_v3v3(pt_relvel, collider_norm);

				VECADDMUL ( i1, collider_norm, pt_magvel * 0.25f );
				cloth1->verts[collpair->ap1].impulse_count++;

				sub_v3_v3v3(pt_relvel, collmd->current_v[collpair->bp2].co, cloth1->verts[collpair->ap2].tv);
				pt_magvel = dot_v3v3(pt_relvel, collider_norm);

				VECADDMUL ( i2, collider_norm, pt_magvel * 0.25f );
				cloth1->verts[collpair->ap2].impulse_count++;

				sub_v3_v3v3(pt_relvel, collmd->current_v[collpair->bp3].co, cloth1->verts[collpair->ap3].tv);
				pt_magvel = dot_v3v3(pt_relvel, collider_norm);

				VECADDMUL ( i3, collider_norm, pt_magvel * 0.25f );
				cloth1->verts[collpair->ap3].impulse_count++;
			}
			else {
				VECADDMUL ( i1, collider_norm, w1 * impulse );
				cloth1->verts[collpair->ap1].impulse_count++;

				VECADDMUL ( i2, collider_norm, w2 * impulse );
				cloth1->verts[collpair->ap2].impulse_count++;

				VECADDMUL ( i3, collider_norm, w3 * impulse );
				cloth1->verts[collpair->ap3].impulse_count++;
			}

			/* Apply repulse impulse if distance too short
			 * I_r = -min(dt*kd, m(0, 1d/dt - v_n))
			 * DG: this formula ineeds to be changed for this code since we apply impulses/repulses like this:
			 * v += impulse; x_new = x + v;
			 * We don't use dt!!
			 * DG TODO: Fix usage of dt here! */
			spf = (float)clmd->sim_parms->stepsPerFrame / clmd->sim_parms->timescale;

			if (backside) {
				d = clmd->coll_parms->epsilon*8.0f/9.0f + epsilon2*8.0f/9.0f;
			}
			else {
				d = clmd->coll_parms->epsilon*8.0f/9.0f + epsilon2*8.0f/9.0f - collpair->distance;
			}

			if ( ( magrelVel < 0.1f*d*spf ) && ( d > ALMOST_ZERO ) ) {
				repulse = MIN2 ( d*1.0f/spf, 0.1f*d*spf - magrelVel );

				/* stay on the safe side and clamp repulse */
				if ( impulse > ALMOST_ZERO )
					repulse = min_ff( repulse, 5.0*impulse );

				/*impulse = repulse / ( 1.0f + w1*w1 + w2*w2 + w3*w3 ); original 2.0 / 0.25 */

				/* Impulse should be uniform throughout polygon, the scaling used above was wrong */
				impulse = repulse / 1.5f;

				VECADDMUL ( i1, collider_norm,  impulse );
				VECADDMUL ( i2, collider_norm,  impulse );
				VECADDMUL ( i3, collider_norm,  impulse );
			}

			result = 1;
		}
		else {
			/* Apply repulse impulse if distance too short
			 * I_r = -min(dt*kd, max(0, 1d/dt - v_n))
			 * DG: this formula ineeds to be changed for this code since we apply impulses/repulses like this:
			 * v += impulse; x_new = x + v;
			 * We don't use dt!! */
			float spf = (float)clmd->sim_parms->stepsPerFrame / clmd->sim_parms->timescale;
			float d;

			if (backside) {
				d = clmd->coll_parms->epsilon*8.0f/9.0f + epsilon2*8.0f/9.0f;
			}
			else {
				d = clmd->coll_parms->epsilon*8.0f/9.0f + epsilon2*8.0f/9.0f - (float)collpair->distance;
			}

			if ( d > ALMOST_ZERO) {
				/* stay on the safe side and clamp repulse */
				float repulse = d*1.0f/spf;

				/*float impulse = repulse / ( 3.0f * ( 1.0f + w1*w1 + w2*w2 + w3*w3 )); original 2.0 / 0.25 */

				/* Impulse should be uniform throughout polygon, the scaling used above was wrong */
				float impulse = repulse / 4.5f;

				if (backside) {
					VECADDMUL ( i1, collider_norm,  impulse );
					VECADDMUL ( i2, collider_norm,  impulse );
					VECADDMUL ( i3, collider_norm,  impulse );
				}
				else {
					VECADDMUL ( i1, collider_norm, w1 * impulse );
					VECADDMUL ( i2, collider_norm, w2 * impulse );
					VECADDMUL ( i3, collider_norm, w3 * impulse );
				}

				cloth1->verts[collpair->ap1].impulse_count++;
				cloth1->verts[collpair->ap2].impulse_count++;
				cloth1->verts[collpair->ap3].impulse_count++;

				result = 1;
			}
		}

		if (result) {
			int i = 0;

			for (i = 0; i < 3; i++) {
				if (cloth1->verts[collpair->ap1].impulse_count > 0 && ABS(cloth1->verts[collpair->ap1].impulse[i]) < ABS(i1[i]))
					cloth1->verts[collpair->ap1].impulse[i] = i1[i];

				if (cloth1->verts[collpair->ap2].impulse_count > 0 && ABS(cloth1->verts[collpair->ap2].impulse[i]) < ABS(i2[i]))
					cloth1->verts[collpair->ap2].impulse[i] = i2[i];

				if (cloth1->verts[collpair->ap3].impulse_count > 0 && ABS(cloth1->verts[collpair->ap3].impulse[i]) < ABS(i3[i]))
					cloth1->verts[collpair->ap3].impulse[i] = i3[i];
			}
		}
	}
	return result;
}

static int cloth_selfcollision_response_static (ClothModifierData *clmd, CollPair *collpair, CollPair *collision_end)
{
	int result = 0;
	Cloth *cloth1;
	float w1, w2, w3, u1, u2, u3;
	float v1[3], v2[3], relativeVelocity[3];
	float magrelVel;

	cloth1 = clmd->clothObject;

	for ( ; collpair != collision_end; collpair++ ) {
		float i1[3], i2[3], i3[3], j1[3], j2[3], j3[3]; /* i are impulses for ap and j are impulses for bp */

		zero_v3(i1);
		zero_v3(i2);
		zero_v3(i3);

		/* only handle static collisions here */
		if ( collpair->flag & COLLISION_IN_FUTURE )
			continue;

		/* compute barycentric coordinates for both collision points */
		collision_compute_barycentric ( collpair->pa,
			cloth1->verts[collpair->ap1].tx,
			cloth1->verts[collpair->ap2].tx,
			cloth1->verts[collpair->ap3].tx,
			&w1, &w2, &w3 );

		/* was: txold */
		collision_compute_barycentric ( collpair->pb,
			cloth1->verts[collpair->bp1].tx,
			cloth1->verts[collpair->bp2].tx,
			cloth1->verts[collpair->bp3].tx,
			&u1, &u2, &u3 );

		/* Calculate relative "velocity". */
		collision_interpolateOnTriangle ( v1, cloth1->verts[collpair->ap1].tv, cloth1->verts[collpair->ap2].tv, cloth1->verts[collpair->ap3].tv, w1, w2, w3 );

		collision_interpolateOnTriangle ( v2, cloth1->verts[collpair->bp1].tv, cloth1->verts[collpair->bp2].tv, cloth1->verts[collpair->bp3].tv, u1, u2, u3 );

		sub_v3_v3v3(relativeVelocity, v2, v1);

		/* Calculate the normal component of the relative velocity (actually only the magnitude - the direction is stored in 'normal'). */
		magrelVel = dot_v3v3(relativeVelocity, collpair->normal);

		/* printf("magrelVel: %f\n", magrelVel); */

		/* TODO: Impulses should be weighed by mass as this is self col,
		 * this has to be done after voronoi mass distribution is implemented */

		/* If v_n_mag < 0 the edges are approaching each other. */
		if ( magrelVel > ALMOST_ZERO ) {
			/* Calculate Impulse magnitude to stop all motion in normal direction. */
			float magtangent = 0, repulse = 0, d = 0;
			double impulse = 0.0;
			float vrel_t_pre[3];
			float temp[3], spf;

			/* calculate tangential velocity */
			copy_v3_v3 ( temp, collpair->normal );
			mul_v3_fl(temp, magrelVel);
			sub_v3_v3v3(vrel_t_pre, relativeVelocity, temp);

			/* Decrease in magnitude of relative tangential velocity due to coulomb friction
			 * in original formula "magrelVel" should be the "change of relative velocity in normal direction" */
			magtangent = min_ff(clmd->coll_parms->friction * 0.01f * magrelVel, len_v3(vrel_t_pre));

			/* Apply friction impulse. */
			if ( magtangent > ALMOST_ZERO ) {
				normalize_v3(vrel_t_pre);

				/*impulse = magtangent / ( 1.0f + w1*w1 + w2*w2 + w3*w3 );  2.0 * */

				/* Impulse should be uniform throughout polygon, the scaling used above was wrong */
				impulse = magtangent / 1.5;

				VECADDMUL ( i1, vrel_t_pre, w1 * impulse );
				VECADDMUL ( i2, vrel_t_pre, w2 * impulse );
				VECADDMUL ( i3, vrel_t_pre, w3 * impulse );
			}

			/* Apply velocity stopping impulse
			 * I_c = m * v_N / 2.0
			 * no 2.0 * magrelVel normally, but looks nicer DG */
			/*impulse =  magrelVel / ( 1.0 + w1*w1 + w2*w2 + w3*w3 );*/

			/* Impulse should be uniform throughout polygon, the scaling used above was wrong */
			impulse =  magrelVel / 1.5f / 2.0f;

			VECADDMUL ( i1, collpair->normal, w1 * impulse );
			cloth1->verts[collpair->ap1].impulse_count++;

			VECADDMUL ( i2, collpair->normal, w2 * impulse );
			cloth1->verts[collpair->ap2].impulse_count++;

			VECADDMUL ( i3, collpair->normal, w3 * impulse );
			cloth1->verts[collpair->ap3].impulse_count++;

			/* Apply repulse impulse if distance too short
			 * I_r = -min(dt*kd, m(0, 1d/dt - v_n))
			 * DG: this formula ineeds to be changed for this code since we apply impulses/repulses like this:
			 * v += impulse; x_new = x + v;
			 * We don't use dt!!
			 * DG TODO: Fix usage of dt here! */
			spf = (float)clmd->sim_parms->stepsPerFrame / clmd->sim_parms->timescale;

			d = clmd->coll_parms->epsilon*8.0f/9.0f * 2.0f - collpair->distance;

			if ( ( magrelVel < 0.1f*d*spf ) && ( d > ALMOST_ZERO ) ) {
				repulse = MIN2 ( d*1.0f/spf, 0.1f*d*spf - magrelVel );

				/* stay on the safe side and clamp repulse */
				/*if ( impulse > ALMOST_ZERO )
					repulse = min_ff( repulse, 2.0*impulse );*/
				repulse = min_ff(impulse, repulse);

				/*impulse = repulse / ( 1.0f + w1*w1 + w2*w2 + w3*w3 ); original 2.0 / 0.25 */

				/* Impulse should be uniform throughout polygon, the scaling used above was wrong */
				impulse = repulse; /* TODO: This might have to be divided by two for self col (to be evaluated) */

				VECADDMUL ( i1, collpair->normal, w1 * impulse );
				VECADDMUL ( i2, collpair->normal, w2 * impulse );
				VECADDMUL ( i3, collpair->normal, w3 * impulse );
			}

			result = 1;
		}
		else {
			/* Apply repulse impulse if distance too short
			 * I_r = -min(dt*kd, max(0, 1d/dt - v_n))
			 * DG: this formula ineeds to be changed for this code since we apply impulses/repulses like this:
			 * v += impulse; x_new = x + v;
			 * We don't use dt!! */
			float spf = (float)clmd->sim_parms->stepsPerFrame / clmd->sim_parms->timescale;
			float d;

			d = clmd->coll_parms->epsilon*8.0f/9.0f * 2.0f - (float)collpair->distance;

			if ( d > ALMOST_ZERO) {
				/* stay on the safe side and clamp repulse */
				float repulse = d*1.0f/spf;

				/*float impulse = repulse / ( 3.0f * ( 1.0f + w1*w1 + w2*w2 + w3*w3 )); original 2.0 / 0.25 */

				/* Impulse should be uniform throughout polygon, the scaling used above was wrong */
				float impulse = repulse / 4.5f / 2.0f; /* TODO: This might have to be divided by two for self col (to be evaluated) */

				VECADDMUL ( i1, collpair->normal, w1 * impulse );
				VECADDMUL ( i2, collpair->normal, w2 * impulse );
				VECADDMUL ( i3, collpair->normal, w3 * impulse );

				cloth1->verts[collpair->ap1].impulse_count++;
				cloth1->verts[collpair->ap2].impulse_count++;
				cloth1->verts[collpair->ap3].impulse_count++;

				result = 1;
			}
		}

		if (result) {
			int i = 0;

			for (i = 0; i < 3; i++) {
				if (cloth1->verts[collpair->ap1].impulse_count > 0 && ABS(cloth1->verts[collpair->ap1].impulse[i]) < ABS(i1[i]))
					cloth1->verts[collpair->ap1].impulse[i] = i1[i];

				if (cloth1->verts[collpair->ap2].impulse_count > 0 && ABS(cloth1->verts[collpair->ap2].impulse[i]) < ABS(i2[i]))
					cloth1->verts[collpair->ap2].impulse[i] = i2[i];

				if (cloth1->verts[collpair->ap3].impulse_count > 0 && ABS(cloth1->verts[collpair->ap3].impulse[i]) < ABS(i3[i]))
					cloth1->verts[collpair->ap3].impulse[i] = i3[i];
			}
		}
	}
	return result;
}

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

//Determines collisions on overlap, collisions are written to collpair[i] and collision+number_collision_found is returned
static CollPair* cloth_collision(ModifierData *md1, ModifierData *md2,
                                 BVHTreeOverlap *overlap, CollPair *collpair, float UNUSED(dt))
{
	ClothModifierData *clmd = (ClothModifierData *)md1;
	CollisionModifierData *collmd = (CollisionModifierData *) md2;
	/* Cloth *cloth = clmd->clothObject; */ /* UNUSED */
	const MVertTri *tri_a, *tri_b;
#ifdef WITH_BULLET
	ClothVertex *verts1 = clmd->clothObject->verts;
#endif
	double distance = 0;
	float epsilon1 = clmd->coll_parms->epsilon;
	float epsilon2 = BLI_bvhtree_get_epsilon ( collmd->bvhtree );

	tri_a = &clmd->clothObject->tri[overlap->indexA];
	tri_b = &collmd->tri[overlap->indexB];

	/* fill face_a */
	collpair->ap1 = tri_a->tri[0];
	collpair->ap2 = tri_a->tri[1];
	collpair->ap3 = tri_a->tri[2];

	/* fill face_b */
	collpair->bp1 = tri_b->tri[0];
	collpair->bp2 = tri_b->tri[1];
	collpair->bp3 = tri_b->tri[2];

	{

#ifdef WITH_BULLET
		// calc distance + normal
		distance = plNearestPoints (
			verts1[collpair->ap1].tx, verts1[collpair->ap2].tx, verts1[collpair->ap3].tx, collmd->current_x[collpair->bp1].co, collmd->current_x[collpair->bp2].co, collmd->current_x[collpair->bp3].co, collpair->pa, collpair->pb, collpair->vector );
#else
		// just be sure that we don't add anything
		distance = 2.0 * (double)( epsilon1 + epsilon2 + ALMOST_ZERO );
#endif

		// distance -1 means no collision result
		if (distance != -1.0 && (distance <= (double)(epsilon1 + epsilon2 + ALMOST_ZERO))) {
			normalize_v3_v3(collpair->normal, collpair->vector);

			collpair->distance = distance;
			collpair->flag = 0;
			collpair++;
		}/*
		else {
			float w1, w2, w3, u1, u2, u3;
			float v1[3], v2[3], relativeVelocity[3];

			// calc relative velocity
			
			// compute barycentric coordinates for both collision points
			collision_compute_barycentric ( collpair->pa,
			verts1[collpair->ap1].txold,
			verts1[collpair->ap2].txold,
			verts1[collpair->ap3].txold,
			&w1, &w2, &w3 );

			// was: txold
			collision_compute_barycentric ( collpair->pb,
			collmd->current_x[collpair->bp1].co,
			collmd->current_x[collpair->bp2].co,
			collmd->current_x[collpair->bp3].co,
			&u1, &u2, &u3 );

			// Calculate relative "velocity".
			collision_interpolateOnTriangle ( v1, verts1[collpair->ap1].tv, verts1[collpair->ap2].tv, verts1[collpair->ap3].tv, w1, w2, w3 );

			collision_interpolateOnTriangle ( v2, collmd->current_v[collpair->bp1].co, collmd->current_v[collpair->bp2].co, collmd->current_v[collpair->bp3].co, u1, u2, u3 );

			sub_v3_v3v3(relativeVelocity, v2, v1);

			if (sqrt(dot_v3v3(relativeVelocity, relativeVelocity)) >= distance)
			{
				// check for collision in the future
				collpair->flag |= COLLISION_IN_FUTURE;
				collpair++;
			}
		}*/
	}
	return collpair;
}

static CollPair* cloth_selfcollision(ModifierData *md1, BVHTreeOverlap *overlap, CollPair *collpair)
{
	/* TODO: This stuff is using Bullet for collision checking,
	 * should be replaced by a better col check function (as well as the obj col function above) */
#ifndef WITH_BULLET
	return collpair;
#endif
	ClothModifierData *clmd = (ClothModifierData *)md1;
	const MVertTri *tri_a, *tri_b;
	ClothVertex *verts1 = clmd->clothObject->verts;
	double distance = 0;
	float epsilon = clmd->coll_parms->epsilon;

	tri_a = &clmd->clothObject->tri[overlap->indexA];
	tri_b = &clmd->clothObject->tri[overlap->indexB];

	for (unsigned int i = 0; i < 3; i++) {
		for (unsigned int j = 0; j < 3; j++) {
			if (tri_a->tri[i] == tri_b->tri[j]) {
				return collpair;
			}
		}
	}

	/* fill face_a */
	collpair->ap1 = tri_a->tri[0];
	collpair->ap2 = tri_a->tri[1];
	collpair->ap3 = tri_a->tri[2];

	/* fill face_b */
	collpair->bp1 = tri_b->tri[0];
	collpair->bp2 = tri_b->tri[1];
	collpair->bp3 = tri_b->tri[2];

	{
		// calc distance + normal
		distance = plNearestPoints(verts1[collpair->ap1].tx, verts1[collpair->ap2].tx, verts1[collpair->ap3].tx,
		                           verts1[collpair->bp1].tx, verts1[collpair->bp2].tx, verts1[collpair->bp3].tx,
		                           collpair->pa, collpair->pb, collpair->vector);

		// distance -1 means no collision result
		if (distance != -1.0 && (distance <= (double)(epsilon * 2.0f + ALMOST_ZERO))) {
			normalize_v3_v3(collpair->normal, collpair->vector);

			collpair->distance = distance;
			collpair->flag = 0;
			collpair++;
		}
	}

	return collpair;
}

static void add_collision_object(Object ***objs, unsigned int *numobj, unsigned int *maxobj, Object *ob, Object *self, int level, unsigned int modifier_type)
{
	CollisionModifierData *cmd= NULL;

	if (ob == self)
		return;

	/* only get objects with collision modifier */
	if (((modifier_type == eModifierType_Collision) && ob->pd && ob->pd->deflect) || (modifier_type != eModifierType_Collision))
		cmd= (CollisionModifierData *)modifiers_findByType(ob, modifier_type);
	
	if (cmd) {
		/* extend array */
		if (*numobj >= *maxobj) {
			*maxobj *= 2;
			*objs= MEM_reallocN(*objs, sizeof(Object *)*(*maxobj));
		}
		
		(*objs)[*numobj] = ob;
		(*numobj)++;
	}

	/* objects in dupli groups, one level only for now */
	if (ob->dup_group && level == 0) {
		GroupObject *go;
		Group *group= ob->dup_group;

		/* add objects */
		for (go= group->gobject.first; go; go= go->next)
			add_collision_object(objs, numobj, maxobj, go->ob, self, level+1, modifier_type);
	}
}

// return all collision objects in scene
// collision object will exclude self 
Object **get_collisionobjects_ext(Scene *scene, Object *self, Group *group, int layer, unsigned int *numcollobj, unsigned int modifier_type, bool dupli)
{
	Base *base;
	Object **objs;
	GroupObject *go;
	unsigned int numobj= 0, maxobj= 100;
	int level = dupli ? 0 : 1;
	
	objs= MEM_callocN(sizeof(Object *)*maxobj, "CollisionObjectsArray");

	/* gather all collision objects */
	if (group) {
		/* use specified group */
		for (go= group->gobject.first; go; go= go->next)
			add_collision_object(&objs, &numobj, &maxobj, go->ob, self, level, modifier_type);
	}
	else {
		Scene *sce_iter;
		/* add objects in same layer in scene */
		for (SETLOOPER(scene, sce_iter, base)) {
			if ( base->lay & layer )
				add_collision_object(&objs, &numobj, &maxobj, base->object, self, level, modifier_type);

		}
	}

	*numcollobj= numobj;

	return objs;
}

Object **get_collisionobjects(Scene *scene, Object *self, Group *group, unsigned int *numcollobj, unsigned int modifier_type)
{
	/* Need to check for active layers, too.
	   Otherwise this check fails if the objects are not on the same layer - DG */
	return get_collisionobjects_ext(scene, self, group, self->lay | scene->lay, numcollobj, modifier_type, true);
}

static void add_collider_cache_object(ListBase **objs, Object *ob, Object *self, int level)
{
	CollisionModifierData *cmd= NULL;
	ColliderCache *col;

	if (ob == self)
		return;

	if (ob->pd && ob->pd->deflect)
		cmd =(CollisionModifierData *)modifiers_findByType(ob, eModifierType_Collision);
	
	if (cmd && cmd->bvhtree) {
		if (*objs == NULL)
			*objs = MEM_callocN(sizeof(ListBase), "ColliderCache array");

		col = MEM_callocN(sizeof(ColliderCache), "ColliderCache");
		col->ob = ob;
		col->collmd = cmd;
		/* make sure collider is properly set up */
		collision_move_object(cmd, 1.0, 0.0);
		BLI_addtail(*objs, col);
	}

	/* objects in dupli groups, one level only for now */
	if (ob->dup_group && level == 0) {
		GroupObject *go;
		Group *group= ob->dup_group;

		/* add objects */
		for (go= group->gobject.first; go; go= go->next)
			add_collider_cache_object(objs, go->ob, self, level+1);
	}
}

ListBase *get_collider_cache(Scene *scene, Object *self, Group *group)
{
	GroupObject *go;
	ListBase *objs= NULL;
	
	/* add object in same layer in scene */
	if (group) {
		for (go= group->gobject.first; go; go= go->next)
			add_collider_cache_object(&objs, go->ob, self, 0);
	}
	else {
		Scene *sce_iter;
		Base *base;

		/* add objects in same layer in scene */
		for (SETLOOPER(scene, sce_iter, base)) {
			if (!self || (base->lay & self->lay))
				add_collider_cache_object(&objs, base->object, self, 0);

		}
	}

	return objs;
}

void free_collider_cache(ListBase **colliders)
{
	if (*colliders) {
		BLI_freelistN(*colliders);
		MEM_freeN(*colliders);
		*colliders = NULL;
	}
}


static void cloth_bvh_objcollisions_nearcheck ( ClothModifierData * clmd, CollisionModifierData *collmd,
	CollPair **collisions, CollPair **collisions_index, int numresult, BVHTreeOverlap *overlap, double dt)
{
	int i;
	
	*collisions = (CollPair *) MEM_mallocN(sizeof(CollPair) * numresult * 4, "collision array" ); // * 4 since cloth_collision_static can return more than 1 collision
	*collisions_index = *collisions;

	for ( i = 0; i < numresult; i++ ) {
		*collisions_index = cloth_collision((ModifierData *)clmd, (ModifierData *)collmd,
		                                    overlap+i, *collisions_index, dt);
	}
}

static void cloth_bvh_selfcollisions_nearcheck (ClothModifierData * clmd, CollPair **collisions, CollPair **collisions_index,
                                                int numresult, BVHTreeOverlap *overlap)
{
	int i;

	*collisions = (CollPair *) MEM_mallocN(sizeof(CollPair) * numresult * 4, "collision array" ); // * 4 since cloth_collision_static can return more than 1 collision
	*collisions_index = *collisions;

	for ( i = 0; i < numresult; i++ ) {
		*collisions_index = cloth_selfcollision((ModifierData *)clmd, overlap+i, *collisions_index);
	}
}

static int cloth_bvh_objcollisions_resolve (ClothModifierData * clmd, CollisionModifierData *collmd,
                                            CollPair *collisions, CollPair *collisions_index, float friction)
{
	Cloth *cloth = clmd->clothObject;
	int i=0, j = 0, /*numfaces = 0, */ mvert_num = 0;
	ClothVertex *verts = NULL;
	int ret = 0;
	int result = 0;

	mvert_num = clmd->clothObject->mvert_num;
	verts = cloth->verts;
	
	// process all collisions (calculate impulses, TODO: also repulses if distance too short)
	result = 1;
	for ( j = 0; j < 2; j++ ) { /* 5 is just a value that ensures convergence */
		result = 0;

		if ( collmd->bvhtree ) {
			result += cloth_collision_response_static (clmd, collmd, collisions, collisions_index, friction);

			// apply impulses in parallel
			if (result) {
				for (i = 0; i < mvert_num; i++) {
					// calculate "velocities" (just xnew = xold + v; no dt in v)
					if (verts[i].impulse_count) {
						// VECADDMUL ( verts[i].tv, verts[i].impulse, 1.0f / verts[i].impulse_count );
						VECADD ( verts[i].tv, verts[i].tv, verts[i].impulse);
						VECADD ( verts[i].dcvel, verts[i].dcvel, verts[i].impulse);
						zero_v3(verts[i].impulse);
						verts[i].impulse_count = 0;

						ret++;
					}
				}
			}
		}

		if (!result) {
			break;
		}
	}
	return ret;
}

static int cloth_bvh_selfcollisions_resolve (ClothModifierData * clmd, CollPair *collisions, CollPair *collisions_index)
{
	Cloth *cloth = clmd->clothObject;
	int i=0, j = 0, /*numfaces = 0, */ mvert_num = 0;
	ClothVertex *verts = NULL;
	int ret = 0;
	int result = 0;

	mvert_num = clmd->clothObject->mvert_num;
	verts = cloth->verts;

	for ( j = 0; j < 2; j++ ) { /* 5 is just a value that ensures convergence */
		result = 0;

		result += cloth_selfcollision_response_static ( clmd, collisions, collisions_index );

		// apply impulses in parallel
		if (result) {
			for (i = 0; i < mvert_num; i++) {
				// calculate "velocities" (just xnew = xold + v; no dt in v)
				if (verts[i].impulse_count) {
					// VECADDMUL ( verts[i].tv, verts[i].impulse, 1.0f / verts[i].impulse_count );
					VECADD ( verts[i].tv, verts[i].tv, verts[i].impulse);
					VECADD ( verts[i].dcvel, verts[i].dcvel, verts[i].impulse);
					zero_v3(verts[i].impulse);
					verts[i].impulse_count = 0;

					ret++;
				}
			}
		}

		if (!result) {
			break;
		}
	}
	return ret;
}

// cloth - object collisions
int cloth_bvh_objcollision(Object *ob, ClothModifierData *clmd, float step, float dt )
{
	Cloth *cloth= clmd->clothObject;
	BVHTree *cloth_bvh= cloth->bvhtree;
	unsigned int i=0, /* numfaces = 0, */ /* UNUSED */ mvert_num = 0, k, l, j;
	int rounds = 0; // result counts applied collisions; ic is for debug output;
	ClothVertex *verts = NULL;
	int ret = 0, ret2 = 0;
	Object **collobjs = NULL;
	unsigned int numcollobj = 0;

	if ((clmd->sim_parms->flags & CLOTH_SIMSETTINGS_FLAG_COLLOBJ) || cloth_bvh==NULL)
		return 0;
	
	verts = cloth->verts;
	/* numfaces = cloth->numfaces; */ /* UNUSED */
	mvert_num = cloth->mvert_num;

	////////////////////////////////////////////////////////////
	// static collisions
	////////////////////////////////////////////////////////////

	if (clmd->coll_parms->flags & CLOTH_COLLSETTINGS_FLAG_ENABLED) {
		bvhtree_update_from_cloth(clmd, 1, false); // 0 means STATIC, 1 means MOVING (see later in this function)
		collobjs = get_collisionobjects(clmd->scene, ob, clmd->coll_parms->group, &numcollobj, eModifierType_Collision);
		
		if (!collobjs)
			return 0;

		/* move object to position (step) in time */
		for (i = 0; i < numcollobj; i++) {
			Object *collob= collobjs[i];
			CollisionModifierData *collmd = (CollisionModifierData *)modifiers_findByType(collob, eModifierType_Collision);

			if (!collmd->bvhtree)
				continue;

			/* move object to position (step) in time */
			collision_move_object ( collmd, step + dt, step );
		}
	}

	if (clmd->coll_parms->flags & CLOTH_COLLSETTINGS_FLAG_SELF) {
		bvhtree_update_from_cloth(clmd, 1, true); // 0 means STATIC, 1 means MOVING (see later in this function)
	}

	do {
		ret2 = 0;

		if (clmd->coll_parms->flags & CLOTH_COLLSETTINGS_FLAG_ENABLED) {
			CollPair **collisions, **collisions_index;

			collisions = MEM_callocN(sizeof(CollPair *) *numcollobj, "CollPair");
			collisions_index = MEM_callocN(sizeof(CollPair *) *numcollobj, "CollPair");

			// check all collision objects
			for (i = 0; i < numcollobj; i++) {
				Object *collob= collobjs[i];
				CollisionModifierData *collmd = (CollisionModifierData *)modifiers_findByType(collob, eModifierType_Collision);
				BVHTreeOverlap *overlap = NULL;
				unsigned int result = 0;

				if (!collmd->bvhtree)
					continue;

				/* search for overlapping collision pairs */
				overlap = BLI_bvhtree_overlap(cloth_bvh, collmd->bvhtree, &result, NULL, NULL);

				// go to next object if no overlap is there
				if ( result && overlap ) {
					/* check if collisions really happen (costly near check) */
					cloth_bvh_objcollisions_nearcheck ( clmd, collmd, &collisions[i],
						&collisions_index[i], result, overlap, dt/(float)clmd->coll_parms->loop_count);

					// resolve nearby collisions
					ret += cloth_bvh_objcollisions_resolve ( clmd, collmd, collisions[i],  collisions_index[i], collob->pd->pdef_cfrict);
					ret2 += ret;
				}

				if ( overlap )
					MEM_freeN ( overlap );
			}

			for (i = 0; i < numcollobj; i++) {
				if ( collisions[i] ) MEM_freeN ( collisions[i] );
			}

			MEM_freeN(collisions);
			MEM_freeN(collisions_index);
		}

		rounds++;

		////////////////////////////////////////////////////////////
		// Test on selfcollisions
		////////////////////////////////////////////////////////////
		if ( clmd->coll_parms->flags & CLOTH_COLLSETTINGS_FLAG_SELF ) {
			/* TODO: This self col stuff is already inside the main col loop,
			 * this inner loop deticated to self col should either just be removed (i.e. run only once),
			 * or moved to outside of the main col loop (to be evaluated later...) */
			for (l = 0; l < (unsigned int)clmd->coll_parms->self_loop_count; l++) {
				/* TODO: add coll quality rounds again */
				BVHTreeOverlap *overlap = NULL;
				unsigned int result = 0;
				CollPair **collisions, **collisions_index;

				collisions = MEM_callocN(sizeof(CollPair *), "CollPair");
				collisions_index = MEM_callocN(sizeof(CollPair *), "CollPair");
	
				// collisions = 1;
				verts = cloth->verts; // needed for openMP
	
				/* numfaces = cloth->numfaces; */ /* UNUSED */
				mvert_num = cloth->mvert_num;
	
				verts = cloth->verts;

				if ( cloth->bvhselftree ) {
					// search for overlapping collision pairs
					overlap = BLI_bvhtree_overlap(cloth->bvhselftree, cloth->bvhselftree, &result, NULL, NULL);

					if (result && overlap) {
						cloth_bvh_selfcollisions_nearcheck (clmd, collisions, collisions_index, result, overlap);

						ret += cloth_bvh_selfcollisions_resolve ( clmd, *collisions,  *collisions_index);
						ret2 += ret;
					}

					if ( overlap )
						MEM_freeN ( overlap );
				}

				if ( *collisions ) MEM_freeN ( *collisions );

				MEM_freeN(collisions);
				MEM_freeN(collisions_index);
			}
		}

		if (clmd->coll_parms->flags & (CLOTH_COLLSETTINGS_FLAG_ENABLED | CLOTH_COLLSETTINGS_FLAG_SELF)) {
			for (i = 0; i < mvert_num; i++) {
				if ( clmd->sim_parms->vgroup_mass>0 ) {
					if ( verts [i].flags & CLOTH_VERT_FLAG_PINNED ) {
						continue;
					}
				}

				VECADD ( verts[i].tx, verts[i].txold, verts[i].tv );
			}
		}
	}
	while ( ret2 && ( clmd->coll_parms->loop_count>rounds ) );
	
	if (collobjs)
		MEM_freeN(collobjs);

	return MIN2 ( ret, 1 );
}

BLI_INLINE void max_v3_v3v3(float r[3], const float a[3], const float b[3])
{
	r[0] = max_ff(a[0], b[0]);
	r[1] = max_ff(a[1], b[1]);
	r[2] = max_ff(a[2], b[2]);
}

void collision_get_collider_velocity(float vel_old[3], float vel_new[3], CollisionModifierData *collmd, CollPair *collpair)
{
	float u1, u2, u3;
	
	/* compute barycentric coordinates */
	collision_compute_barycentric(collpair->pb,
	                              collmd->current_x[collpair->bp1].co,
	                              collmd->current_x[collpair->bp2].co,
	                              collmd->current_x[collpair->bp3].co,
	                              &u1, &u2, &u3);
	
	collision_interpolateOnTriangle(vel_new, collmd->current_v[collpair->bp1].co, collmd->current_v[collpair->bp2].co, collmd->current_v[collpair->bp3].co, u1, u2, u3);
	/* XXX assume constant velocity of the collider for now */
	copy_v3_v3(vel_old, vel_new);
}

BLI_INLINE bool cloth_point_face_collision_params(const float p1[3], const float p2[3], const float v0[3], const float v1[3], const float v2[3],
                                                  float r_nor[3], float *r_lambda, float r_w[4])
{
	float edge1[3], edge2[3], p2face[3], p1p2[3], v0p2[3];
	float nor_v0p2, nor_p1p2;
	
	sub_v3_v3v3(edge1, v1, v0);
	sub_v3_v3v3(edge2, v2, v0);
	cross_v3_v3v3(r_nor, edge1, edge2);
	normalize_v3(r_nor);
	
	nor_v0p2 = dot_v3v3(v0p2, r_nor);
	madd_v3_v3v3fl(p2face, p2, r_nor, -nor_v0p2);
	interp_weights_face_v3(r_w, v0, v1, v2, NULL, p2face);
	
	sub_v3_v3v3(p1p2, p2, p1);
	sub_v3_v3v3(v0p2, p2, v0);
	nor_p1p2 = dot_v3v3(p1p2, r_nor);
	*r_lambda = (nor_p1p2 != 0.0f ? nor_v0p2 / nor_p1p2 : 0.0f);
	
	return r_w[1] >= 0.0f && r_w[2] >= 0.0f && r_w[1] + r_w[2] <= 1.0f;

#if 0 /* XXX this method uses the intersection point, but is broken and doesn't work well in general */
	float p[3], vec1[3], line[3], edge1[3], edge2[3], q[3];
	float a, f, u, v;

	sub_v3_v3v3(edge1, v1, v0);
	sub_v3_v3v3(edge2, v2, v0);
	sub_v3_v3v3(line, p2, p1);

	cross_v3_v3v3(p, line, edge2);
	a = dot_v3v3(edge1, p);
	if (a == 0.0f) return 0;
	f = 1.0f / a;

	sub_v3_v3v3(vec1, p1, v0);

	u = f * dot_v3v3(vec1, p);
	if ((u < 0.0f) || (u > 1.0f))
		return false;

	cross_v3_v3v3(q, vec1, edge1);

	v = f * dot_v3v3(line, q);
	if ((v < 0.0f) || ((u + v) > 1.0f))
		return false;

	*r_lambda = f * dot_v3v3(edge2, q);
	/* don't care about 0..1 lambda range here */
	/*if ((*r_lambda < 0.0f) || (*r_lambda > 1.0f))
	 *	return 0;
	 */

	r_w[0] = 1.0f - u - v;
	r_w[1] = u;
	r_w[2] = v;
	r_w[3] = 0.0f;

	cross_v3_v3v3(r_nor, edge1, edge2);
	normalize_v3(r_nor);

	return true;
#endif
}

static CollPair *cloth_point_collpair(
        float p1[3], float p2[3], const MVert *mverts, int bp1, int bp2, int bp3,
        int index_cloth, int index_coll, float epsilon, CollPair *collpair)
{
	const float *co1 = mverts[bp1].co, *co2 = mverts[bp2].co, *co3 = mverts[bp3].co;
	float lambda /*, distance1 */, distance2;
	float facenor[3], v1p1[3], v1p2[3];
	float w[4];

	if (!cloth_point_face_collision_params(p1, p2, co1, co2, co3, facenor, &lambda, w))
		return collpair;
	
	sub_v3_v3v3(v1p1, p1, co1);
//	distance1 = dot_v3v3(v1p1, facenor);
	sub_v3_v3v3(v1p2, p2, co1);
	distance2 = dot_v3v3(v1p2, facenor);
//	if (distance2 > epsilon || (distance1 < 0.0f && distance2 < 0.0f))
	if (distance2 > epsilon)
		return collpair;
	
	collpair->face1 = index_cloth; /* XXX actually not a face, but equivalent index for point */
	collpair->face2 = index_coll;
	collpair->ap1 = index_cloth;
	collpair->ap2 = collpair->ap3 = -1; /* unused */
	collpair->bp1 = bp1;
	collpair->bp2 = bp2;
	collpair->bp3 = bp3;
	
	/* note: using the second point here, which is
	 * the current updated position that needs to be corrected
	 */
	copy_v3_v3(collpair->pa, p2);
	collpair->distance = distance2;
	mul_v3_v3fl(collpair->vector, facenor, -distance2);
	
	interp_v3_v3v3v3(collpair->pb, co1, co2, co3, w);
	
	copy_v3_v3(collpair->normal, facenor);
	collpair->time = lambda;
	collpair->flag = 0;
	
	collpair++;
	return collpair;
}

//Determines collisions on overlap, collisions are written to collpair[i] and collision+number_collision_found is returned
static CollPair *cloth_point_collision(
        ModifierData *md1, ModifierData *md2,
        BVHTreeOverlap *overlap, float epsilon, CollPair *collpair, float UNUSED(dt))
{
	ClothModifierData *clmd = (ClothModifierData *)md1;
	CollisionModifierData *collmd = (CollisionModifierData *) md2;
	/* Cloth *cloth = clmd->clothObject; */ /* UNUSED */
	ClothVertex *vert = NULL;
	const MVertTri *vt;
	const MVert *mverts = collmd->current_x;

	vert = &clmd->clothObject->verts[overlap->indexA];
	vt = &collmd->tri[overlap->indexB];

	collpair = cloth_point_collpair(
	        vert->tx, vert->x, mverts,
	        vt->tri[0], vt->tri[1], vt->tri[2],
	        overlap->indexA, overlap->indexB,
	        epsilon, collpair);

	return collpair;
}

static void cloth_points_objcollisions_nearcheck(ClothModifierData * clmd, CollisionModifierData *collmd,
                                                     CollPair **collisions, CollPair **collisions_index,
                                                     int numresult, BVHTreeOverlap *overlap, float epsilon, double dt)
{
	int i;
	
	/* can return 2 collisions in total */
	*collisions = (CollPair *) MEM_mallocN(sizeof(CollPair) * numresult * 2, "collision array" );
	*collisions_index = *collisions;

	for ( i = 0; i < numresult; i++ ) {
		*collisions_index = cloth_point_collision((ModifierData *)clmd, (ModifierData *)collmd,
		                                          overlap+i, epsilon, *collisions_index, dt);
	}
}

void cloth_find_point_contacts(Object *ob, ClothModifierData *clmd, float step, float dt,
                               ColliderContacts **r_collider_contacts, int *r_totcolliders)
{
	Cloth *cloth= clmd->clothObject;
	BVHTree *cloth_bvh;
	unsigned int i = 0, mvert_num = 0;
	ClothVertex *verts = NULL;
	
	ColliderContacts *collider_contacts;
	
	Object **collobjs = NULL;
	unsigned int numcollobj = 0;
	
	verts = cloth->verts;
	mvert_num = cloth->mvert_num;
	
	////////////////////////////////////////////////////////////
	// static collisions
	////////////////////////////////////////////////////////////
	
	// create temporary cloth points bvh
	cloth_bvh = BLI_bvhtree_new(mvert_num, clmd->coll_parms->epsilon, 4, 6);
	/* fill tree */
	for (i = 0; i < mvert_num; i++) {
		float co[6];
		
		copy_v3_v3(&co[0*3], verts[i].x);
		copy_v3_v3(&co[1*3], verts[i].tx);
		
		BLI_bvhtree_insert(cloth_bvh, i, co, 2);
	}
	/* balance tree */
	BLI_bvhtree_balance(cloth_bvh);
	
	collobjs = get_collisionobjects(clmd->scene, ob, clmd->coll_parms->group, &numcollobj, eModifierType_Collision);
	if (!collobjs) {
		*r_collider_contacts = NULL;
		*r_totcolliders = 0;
		return;
	}
	
	/* move object to position (step) in time */
	for (i = 0; i < numcollobj; i++) {
		Object *collob= collobjs[i];
		CollisionModifierData *collmd = (CollisionModifierData *)modifiers_findByType(collob, eModifierType_Collision);
		if (!collmd->bvhtree)
			continue;
		
		/* move object to position (step) in time */
		collision_move_object ( collmd, step + dt, step );
	}
	
	collider_contacts = MEM_callocN(sizeof(ColliderContacts) * numcollobj, "CollPair");
	
	// check all collision objects
	for (i = 0; i < numcollobj; i++) {
		ColliderContacts *ct = collider_contacts + i;
		Object *collob= collobjs[i];
		CollisionModifierData *collmd = (CollisionModifierData *)modifiers_findByType(collob, eModifierType_Collision);
		BVHTreeOverlap *overlap;
		unsigned int result = 0;
		float epsilon;
		
		ct->ob = collob;
		ct->collmd = collmd;
		ct->collisions = NULL;
		ct->totcollisions = 0;
		
		if (!collmd->bvhtree)
			continue;
		
		/* search for overlapping collision pairs */
		overlap = BLI_bvhtree_overlap(cloth_bvh, collmd->bvhtree, &result, NULL, NULL);
		epsilon = BLI_bvhtree_get_epsilon(collmd->bvhtree);
		
		// go to next object if no overlap is there
		if (result && overlap) {
			CollPair *collisions_index;
			
			/* check if collisions really happen (costly near check) */
			cloth_points_objcollisions_nearcheck(clmd, collmd, &ct->collisions, &collisions_index,
			                                     result, overlap, epsilon, dt);
			ct->totcollisions = (int)(collisions_index - ct->collisions);
			
			// resolve nearby collisions
//			ret += cloth_points_objcollisions_resolve(clmd, collmd, collob->pd, collisions[i], collisions_index[i], dt);
		}
		
		if (overlap)
			MEM_freeN(overlap);
	}
	
	if (collobjs)
		MEM_freeN(collobjs);

	BLI_bvhtree_free(cloth_bvh);
	
	////////////////////////////////////////////////////////////
	// update positions
	// this is needed for bvh_calc_DOP_hull_moving() [kdop.c]
	////////////////////////////////////////////////////////////
	
	// verts come from clmd
	for (i = 0; i < mvert_num; i++) {
		if ( clmd->sim_parms->vgroup_mass>0 ) {
			if (verts [i].flags & CLOTH_VERT_FLAG_PINNED) {
				continue;
			}
		}
		
		VECADD(verts[i].tx, verts[i].txold, verts[i].tv);
	}
	////////////////////////////////////////////////////////////
	
	*r_collider_contacts = collider_contacts;
	*r_totcolliders = numcollobj;
}

void cloth_free_contacts(ColliderContacts *collider_contacts, int totcolliders)
{
	if (collider_contacts) {
		int i;
		for (i = 0; i < totcolliders; ++i) {
			ColliderContacts *ct = collider_contacts + i;
			if (ct->collisions) {
				MEM_freeN(ct->collisions);
			}
		}
		MEM_freeN(collider_contacts);
	}
}
