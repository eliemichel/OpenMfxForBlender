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
 * along with this program; if not, write to the Free Software  Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2005 by the Blender Foundation.
 * All rights reserved.
 *
 * Contributor(s): Your name
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

/** \file blender/modifiers/intern/MOD_Snap.c
 *  \ingroup modifiers
 */


#include "DNA_meshdata_types.h"

#include "BLI_math.h"
#include "BLI_utildefines.h"
#include "BLI_string.h"

#include "MEM_guardedalloc.h"

#include "BKE_cdderivedmesh.h"
#include "BKE_library_query.h"
#include "BKE_particle.h"
#include "BKE_deform.h"

#include "MOD_modifiertypes.h"
#include "MOD_util.h"

#include "depsgraph_private.h"

static void initData(ModifierData *md)
{
	VertexSnapModifierData *smd = (VertexSnapModifierData *) md;
	smd->blend = 1.0f;
	smd->target = NULL;
	smd->vertex_group[0] = 0;
}
 
static void copyData(ModifierData *md, ModifierData *target)
{
	/*
	VertexSnapModifierData *smd = (VertexSnapModifierData *) md;
	VertexSnapModifierData *tsmd = (VertexSnapModifierData *) target;
	*/
	modifier_copyData_generic(md, target);
}
 
static int isDisabled(ModifierData *md, int UNUSED(useRenderParams))
{
	/* disable if modifier there is no connected target object*/
	VertexSnapModifierData *smd = (VertexSnapModifierData *)md;
	return (smd->target == NULL);
}

static CustomDataMask requiredDataMask(Object *UNUSED(ob), ModifierData *md)
{
	VertexSnapModifierData *enmd = (VertexSnapModifierData *)md;
	CustomDataMask dataMask = 0;

	/* Ask for vertexgroups if we need them. */
	if (enmd->vertex_group[0]) {
		dataMask |= ( CD_MASK_MDEFORMVERT );
	}

	return dataMask;
}

static void foreachObjectLink(ModifierData *md, Object *ob, ObjectWalkFunc walk, void *userData)
{
	VertexSnapModifierData *smd = (VertexSnapModifierData *)md;

	walk( userData, ob, &smd->target, IDWALK_NOP );
}

static void updateDepgraph(ModifierData *md, DagForest *forest,
	struct Main *UNUSED(bmain),
	struct Scene *UNUSED(scene),
	Object *UNUSED(ob),
	DagNode *obNode)
{
	VertexSnapModifierData *smd = (VertexSnapModifierData *)md;

	if (smd->target) {
		DagNode *curNode = dag_get_node(forest, smd->target);

		dag_add_relation(forest, curNode, obNode,
				DAG_RL_DATA_DATA, "Surface Deform Modifier");
	}
}

static void updateDepsgraph(ModifierData *md,
	struct Main *UNUSED(bmain),
	struct Scene *UNUSED(scene),
	Object *UNUSED(ob),
	struct DepsNodeHandle *node)
{
	VertexSnapModifierData *smd = (VertexSnapModifierData *)md;
	if (smd->target != NULL) {
		DEG_add_object_relation(node, smd->target,
				DEG_OB_COMP_GEOMETRY, "Surface Deform Modifier");
	}
}

static void SnapModifier_do(
        VertexSnapModifierData *vmd, Object *ob, DerivedMesh *dm,
        float (*vertexCos)[3], int numVerts)
{
	int i, index;
	struct Object *target       = vmd->target;
	DerivedMesh   *target_dm    = NULL;
	MVert         *target_verts = NULL;
	MDeformVert   *dverts       = NULL;
	int deform_group_index      = -1;
	int vertex_count            = numVerts;
	int target_vertex_count     = 0;
	
	float blend = vmd->blend;

	if ( blend == 0.0 || target == NULL )
		return;

	if ( !(target && target != ob && target->type == OB_MESH) ) {
		return;
	}

	target_dm = get_dm_for_modifier( target, MOD_APPLY_RENDER );
	if (!target_dm) {
		return;
	}

	target_vertex_count = target_dm->getNumVerts( target_dm );
	if (vertex_count < target_vertex_count) {
		vertex_count = target_vertex_count;
	}

	target_verts = CDDM_get_verts( target_dm );
	modifier_get_vgroup( ob, dm, vmd->vertex_group, &dverts, &deform_group_index );

	for ( int index=0; index < vertex_count; index++ ) {
		float final_blend = blend;
		if (dverts) {
			final_blend *= defvert_find_weight( &dverts[index], deform_group_index);
		}

		if ( final_blend ) {
			float co_tmp[3];
			// mul_v3_m4v3(co_tmp, hd->mat, co);
			interp_v3_v3v3( vertexCos[index], vertexCos[index], target_verts[index].co, final_blend );
		}
	}

	// for (i = 0; i < numVerts; i++) {
	// 	vertexCos[i][0] = vertexCos[i][0] * blend;
	// 	vertexCos[i][1] = vertexCos[i][1] * blend;
	// 	vertexCos[i][2] = vertexCos[i][2] * blend;
	// }

	if (target_dm) {
		target_dm->release(target_dm);
	}
}
 
static void deformVerts(ModifierData *md, Object *ob, DerivedMesh *derivedData,
                        float (*vertexCos)[3], int numVerts, ModifierApplyFlag UNUSED(flag))
{
	DerivedMesh *dm = get_dm(ob, NULL, derivedData, NULL, false, false);
 
	SnapModifier_do((VertexSnapModifierData *)md, ob, dm,
	                  vertexCos, numVerts);
 
	if (dm != derivedData)
		dm->release(dm);
}
 
static void deformVertsEM(
        ModifierData *md, Object *ob, struct BMEditMesh *editData,
        DerivedMesh *derivedData, float (*vertexCos)[3], int numVerts)
{
	DerivedMesh *dm = get_dm(ob, editData, derivedData, NULL, false, false );
 
	SnapModifier_do((VertexSnapModifierData *)md, ob, dm,
	                  vertexCos, numVerts);
 
	if (dm != derivedData)
		dm->release(dm);
}
 
 
ModifierTypeInfo modifierType_VertexSnap = {
	/* name */              "VertexSnap",
	/* structName */        "VertexSnapModifierData",
	/* structSize */        sizeof(VertexSnapModifierData),
	/* type */              eModifierTypeType_OnlyDeform,
	/* flags */             eModifierTypeFlag_AcceptsMesh |
	                        eModifierTypeFlag_SupportsEditmode,
 
	/* copyData */          copyData,
	/* deformVerts */       deformVerts,
	/* deformMatrices */    NULL,
	/* deformVertsEM */     deformVertsEM,
	/* deformMatricesEM */  NULL,
	/* applyModifier */     NULL,
	/* applyModifierEM */   NULL,
	/* initData */          initData,
	/* requiredDataMask */  requiredDataMask,
	/* freeData */          NULL,
	/* isDisabled */        isDisabled,
	/* updateDepgraph */    updateDepgraph,
	/* updateDepsgraph */   updateDepsgraph,
	/* dependsOnTime */     NULL,
	/* dependsOnNormals */  NULL,
	/* foreachObjectLink */ foreachObjectLink,
	/* foreachIDLink */     NULL,
	/* foreachTexLink */    NULL,
};

