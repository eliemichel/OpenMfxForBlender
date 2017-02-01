# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####

# <pep8 compliant>
import bpy
from bpy.types import Menu, Panel

from bl_ui.properties_physics_common import (
        point_cache_ui,
        effector_weights_ui,
        )


def cloth_panel_enabled(md):
    return md.point_cache.is_baked is False


class CLOTH_MT_presets(Menu):
    bl_label = "Cloth Presets"
    preset_subdir = "cloth"
    preset_operator = "script.execute_preset"
    draw = Menu.draw_preset


class PhysicButtonsPanel:
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "physics"

    @classmethod
    def poll(cls, context):
        ob = context.object
        rd = context.scene.render
        return (ob and ob.type == 'MESH') and (rd.engine in cls.COMPAT_ENGINES) and (context.cloth)


class PHYSICS_PT_cloth(PhysicButtonsPanel, Panel):
    bl_label = "Cloth"
    COMPAT_ENGINES = {'BLENDER_RENDER'}

    def draw(self, context):
        layout = self.layout

        md = context.cloth
        ob = context.object
        cloth = md.settings

        layout.active = cloth_panel_enabled(md)

        split = layout.split(percentage=0.3)

        split.label(text="Presets:")
        sub = split.row(align=True)
        sub.menu("CLOTH_MT_presets", text=bpy.types.CLOTH_MT_presets.bl_label)
        sub.operator("cloth.preset_add", text="", icon='ZOOMIN')
        sub.operator("cloth.preset_add", text="", icon='ZOOMOUT').remove_active = True

        col = layout.column()

        split = col.split(percentage=0.3)
        split.label(text="Quality:")
        split.prop(cloth, "quality", text="Steps")

        split = col.split(percentage=0.3)
        split.label(text="Speed:")
        split.prop(cloth, "time_scale", text="Multiplier")

        layout.separator()

        layout.label("Material Properties:")

        col = layout.column()

        split = col.split(percentage=0.3)
        split.label("Mass:")
        split.prop(cloth, "mass", text="")

        split = col.split(percentage=0.3)
        split.label("Air Viscosity:")
        split.prop(cloth, "air_damping", text="")

        col = layout.column()

        split = col.split(percentage=0.3)
        split.separator()

        row = split.row(align=True)
        row.label("Stiffness:")
        row.label("Damping:")

        split = col.split(percentage=0.3)
        split.label("Tension:")
        row = split.row(align=True)
        row.prop(cloth, "tension_stiffness", text="")
        row.prop(cloth, "tension_damping", text="")

        split = col.split(percentage=0.3)
        split.label("Compression:")
        row = split.row(align=True)
        row.prop(cloth, "compression_stiffness", text="")
        row.prop(cloth, "compression_damping", text="")

        split = col.split(percentage=0.3)
        split.label("Shear:")
        row = split.row(align=True)
        row.prop(cloth, "shear_stiffness", text="")
        row.prop(cloth, "shear_damping", text="")

        split = col.split(percentage=0.3)
        split.label("Bending:")
        row = split.row(align=True)
        row.prop(cloth, "bending_stiffness", text="")
        row.prop(cloth, "bending_damping", text="")

        split = col.split(percentage=0.3)
        split.separator()

        row = split.row(align=True)
        row.label("Plasticity:")
        row.label("Threshold:")

        split = col.split(percentage=0.3)
        split.label("Structural:")
        row = split.row(align=True)
        row.prop(cloth, "structural_plasticity", text="")
        row.prop(cloth, "structural_yield_factor", text="")

        split = col.split(percentage=0.3)
        split.label("Bending:")
        row = split.row(align=True)
        row.prop(cloth, "bending_plasticity", text="")
        row.prop(cloth, "bending_yield_factor", text="")

        # Disabled for now
        """
        if cloth.vertex_group_mass:
            layout.label(text="Goal:")

            col = layout.column_flow()
            col.prop(cloth, "goal_default", text="Default")
            col.prop(cloth, "goal_spring", text="Stiffness")
            col.prop(cloth, "goal_friction", text="Friction")
        """


class PHYSICS_PT_cloth_cache(PhysicButtonsPanel, Panel):
    bl_label = "Cloth Cache"
    bl_options = {'DEFAULT_CLOSED'}
    COMPAT_ENGINES = {'BLENDER_RENDER'}

    def draw(self, context):
        md = context.cloth
        point_cache_ui(self, context, md.point_cache, cloth_panel_enabled(md), 'CLOTH')


class PHYSICS_PT_cloth_shape(PhysicButtonsPanel, Panel):
    bl_label = "Cloth Shape"
    bl_options = {'DEFAULT_CLOSED'}
    COMPAT_ENGINES = {'BLENDER_RENDER'}

    def draw(self, context):
        layout = self.layout

        md = context.cloth
        ob = context.object
        cloth = context.cloth.settings

        layout.active = cloth_panel_enabled(md)

        split = layout.split(percentage=0.3, align=True)

        split.label("Pinning:")

        split.prop_search(cloth, "vertex_group_mass", ob, "vertex_groups", text="")

        sub = split.row(align=True)
        sub.active = cloth.vertex_group_mass != ""
        sub.prop(cloth, "pin_stiffness", text="Stiffness")

        split = layout.split(percentage=0.3)
        split.prop(cloth, "use_sewing_springs", text="Sewing:")

        sub = split.column()
        sub.active = cloth.use_sewing_springs
        sub.prop(cloth, "sewing_force_max", text="Sewing Force")

        row = layout.row()
        row.prop(cloth, "shrinking", text="Shrinking")
        row.prop(cloth, "rest_planarity_factor", text="Flattening")

        layout.prop(cloth, "use_dynamic_mesh", text="Dynamic Mesh")

        key = ob.data.shape_keys

        if key:
            sub = layout.column()
            sub.active = not cloth.use_dynamic_mesh
            sub.prop_search(cloth, "rest_shape_key", key, "key_blocks")


class PHYSICS_PT_cloth_collision(PhysicButtonsPanel, Panel):
    bl_label = "Cloth Collision"
    bl_options = {'DEFAULT_CLOSED'}
    COMPAT_ENGINES = {'BLENDER_RENDER'}

    def draw(self, context):
        layout = self.layout

        cloth = context.cloth.collision_settings
        md = context.cloth
        ob = context.object

        layout.active = cloth_panel_enabled(md)

        layout.prop(cloth, "collision_quality")

        layout.separator()

        col = layout.column()

        col.prop(cloth, "use_collision", text="Object Collision:")

        sub = col.column()
        sub.active = cloth.use_collision
        sub.prop(cloth, "distance_min", slider=True, text="Distance")
        sub.prop(cloth, "group")

        layout.separator()

        col = layout.column()

        split = col.split()

        sub = split.column()
        sub.prop(cloth, "use_self_collision", text="Self Collision:")

        sub = split.column()
        sub.active = cloth.use_self_collision
        sub.prop(cloth, "self_friction", text="Friction")

        sub = col.column()
        sub.active = cloth.use_self_collision
        sub.prop(cloth, "self_distance_min", slider=True, text="Distance")
        sub.prop_search(cloth, "vertex_group_self_collisions", ob, "vertex_groups", text="Vertex Group")


class PHYSICS_PT_cloth_scaling(PhysicButtonsPanel, Panel):
    bl_label = "Cloth Property Scaling"
    bl_options = {'DEFAULT_CLOSED'}
    COMPAT_ENGINES = {'BLENDER_RENDER'}

    def draw(self, context):
        layout = self.layout

        md = context.cloth
        ob = context.object
        cloth = context.cloth.settings

        layout.active = cloth_panel_enabled(md)

        row = layout.row()

        col = row.column(align=True)
        col.label(text="Structural Stiffness:")
        col.label(text="Tension:")
        col.label(text="Compression:")

        col = row.column(align=True)
        col.prop_search(cloth, "vertex_group_structural_stiffness", ob, "vertex_groups", text="")

        sub = col.column(align=True)
        sub.active = cloth.vertex_group_structural_stiffness != ""
        sub.prop(cloth, "tension_stiffness_max", text="Max")
        sub.prop(cloth, "compression_stiffness_max", text="Max")

        split = layout.split(percentage=0.25, align=True)
        split.label(text="Shear:")
        split.prop_search(cloth, "vertex_group_shear_stiffness", ob, "vertex_groups", text="")

        sub = split.row(align=True)
        sub.active = cloth.vertex_group_shear_stiffness != ""
        sub.prop(cloth, "shear_stiffness_max", text="Max")

        split = layout.split(percentage=0.25, align=True)
        split.label("Bending:")
        split.prop_search(cloth, "vertex_group_bending", ob, "vertex_groups", text="")

        sub = split.row(align=True)
        sub.active = cloth.vertex_group_bending != ""
        sub.prop(cloth, "bending_stiffness_max", text="Max")

        split = layout.split(percentage=0.25, align=True)
        split.label("Shrinking:")
        split.prop_search(cloth, "vertex_group_shrink", ob, "vertex_groups", text="")

        sub = split.row(align=True)
        sub.active = cloth.vertex_group_shrink != ""
        sub.prop(cloth, "shrinking_max", text="Max")


class PHYSICS_PT_cloth_adaptive_subframes(PhysicButtonsPanel, Panel):
    bl_label = "Cloth Adaptive Subframes"
    bl_options = {'DEFAULT_CLOSED'}
    COMPAT_ENGINES = {'BLENDER_RENDER'}

    def draw(self, context):
        layout = self.layout

        md = context.cloth
        cloth = context.cloth.settings

        layout.active = cloth_panel_enabled(md)

        layout.prop(cloth, "max_sub_steps")
        layout.separator()

        layout.prop(cloth, "use_adaptive_subframes")

        col = layout.column()
        col.active = cloth.use_adaptive_subframes

        col.prop(cloth, "max_velocity")
        col.prop(cloth, "adjustment_factor")

        layout.prop(cloth, "use_impulse_adaptive_subframes")

        col = layout.column()
        col.active = cloth.use_impulse_adaptive_subframes

        col.prop(cloth, "max_impulse")
        col.prop(cloth, "impulse_adjustment_factor")


class PHYSICS_PT_cloth_field_weights(PhysicButtonsPanel, Panel):
    bl_label = "Cloth Field Weights"
    bl_options = {'DEFAULT_CLOSED'}
    COMPAT_ENGINES = {'BLENDER_RENDER'}

    def draw(self, context):
        cloth = context.cloth.settings
        effector_weights_ui(self, context, cloth.effector_weights, 'CLOTH')

if __name__ == "__main__":  # only for live edit.
    bpy.utils.register_module(__name__)
