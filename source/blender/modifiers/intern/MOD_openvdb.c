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
 * Contributor(s):
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/modifiers/intern/MOD_openvdb.c
 *  \ingroup modifiers
 */

#include <stdio.h>

#include "DNA_scene_types.h"
#include "DNA_object_types.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_smoke_types.h"

#include "BLI_blenlib.h"
#include "BLI_utildefines.h"
#include "BLI_string.h"
#include "BLI_path_util.h"
#include "BLI_math.h"

#include "BKE_DerivedMesh.h"
#include "BKE_scene.h"
#include "BKE_global.h"
#include "BKE_mesh.h"
#include "BKE_main.h"
#include "BKE_smoke.h"
#include "BKE_pointcache.h"

#include "MEM_guardedalloc.h"

#include "MOD_modifiertypes.h"

#include "openvdb_capi.h"

static void initData(ModifierData *md)
{
	OpenVDBModifierData *vdbmd = (OpenVDBModifierData *)md;
	SmokeModifierData *smd = (SmokeModifierData *)modifier_new(eModifierType_Smoke);

	smd->type = MOD_SMOKE_TYPE_DOMAIN;

	smokeModifier_createType(smd);

	smd->domain->cache_file_format = PTCACHE_FILE_OPENVDB_EXTERN;
	smd->domain->vdb = vdbmd;

	vdbmd->smoke = smd;
	vdbmd->grids = NULL;
	vdbmd->numgrids = 0;
}

static void copyData(ModifierData *md, ModifierData *target)
{
	modifier_copyData_generic(md, target);
}

static bool dependsOnTime(ModifierData *UNUSED(md))
{
	return true;
}

static bool isDisabled(ModifierData *md, int UNUSED(useRenderParams))
{
	return false;
	OpenVDBModifierData *vdbmd = (OpenVDBModifierData *) md;

	/* leave it up to the modifier to check the file is valid on calculation */
	return vdbmd->filepath[0] == '\0';
}

static DerivedMesh *applyModifier(ModifierData *md, Object *ob,
                                  DerivedMesh *dm,
                                  ModifierApplyFlag flag)
{
	OpenVDBModifierData *vdbmd = (OpenVDBModifierData*) md;
	ModifierData *smd = (ModifierData *)vdbmd->smoke;

	smd->scene = md->scene;

	MEM_SAFE_FREE(vdbmd->grids);
	vdbmd->numgrids = 0;

	if (BLI_exists(vdbmd->filepath)) {
		struct OpenVDBReader *reader = OpenVDBReader_create();
		OpenVDBReader_open(reader, vdbmd->filepath);

		vdbmd->numgrids = OpenVDB_get_name_array(reader, (char***)&vdbmd->grids);

		OpenVDBReader_free(reader);
	}

	return modwrap_applyModifier(smd, ob, dm, flag);
}


ModifierTypeInfo modifierType_OpenVDB = {
	/* name */              "OpenVDB",
	/* structName */        "OpenVDBModifierData",
	/* structSize */        sizeof(OpenVDBModifierData),
	/* type */              eModifierTypeType_Constructive,
    /* flags */             eModifierTypeFlag_AcceptsMesh |
	                        eModifierTypeFlag_UsesPointCache |
	                        eModifierTypeFlag_Single,

	/* copyData */          copyData,
	/* deformVerts */       NULL,
	/* deformMatrices */    NULL,
	/* deformVertsEM */     NULL,
	/* deformMatricesEM */  NULL,
	/* applyModifier */     applyModifier,
	/* applyModifierEM */   NULL,
	/* initData */          initData,
	/* requiredDataMask */  NULL,
	/* freeData */          NULL,
	/* isDisabled */        isDisabled,
	/* updateDepgraph */    NULL,
	/* updateDepsgraph */   NULL,
	/* dependsOnTime */     dependsOnTime,
	/* dependsOnNormals */  NULL,
	/* foreachObjectLink */ NULL,
	/* foreachIDLink */     NULL,
	/* foreachTexLink */    NULL,
};
