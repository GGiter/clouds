# ##### BEGIN GPL LICENSE BLOCK #####
# Original header details remain unchanged...
# ##### END GPL LICENSE BLOCK #####

import bpy
import os
import json
from bpy_extras.io_utils import ExportHelper

from bpy.props import (BoolProperty,
                       FloatProperty,
                       StringProperty,
                       EnumProperty)

bl_info = {
    "name": "OBJ Batch Export",
    "author": "p2or, brockmann, trippeljojo, modified by ChatGPT",
    "version": (0, 7, 0),
    "blender": (3, 6, 0),
    "location": "File > Import-Export",
    "description": "Export unique OBJ files for shared meshes based on geometry, generate JSON for all objects",
    "doc_url": "https://github.com/p2or/blender-batch-export-wavefront-obj",
    "tracker_url": "https://github.com/p2or/blender-batch-export-wavefront-obj",
    "category": "Import-Export"}


def are_meshes_identical(mesh_a, mesh_b):
    """Compare two meshes to determine if they are geometrically identical."""
    if len(mesh_a.vertices) != len(mesh_b.vertices) or len(mesh_a.edges) != len(mesh_b.edges) or len(mesh_a.polygons) != len(mesh_b.polygons):
        return False

    # Compare vertex positions
    for vert_a, vert_b in zip(mesh_a.vertices, mesh_b.vertices):
        if (vert_a.co - vert_b.co).length > 1e-6:
            return False

    # Compare edges
    edges_a = {tuple(sorted(edge.vertices)) for edge in mesh_a.edges}
    edges_b = {tuple(sorted(edge.vertices)) for edge in mesh_b.edges}
    if edges_a != edges_b:
        return False

    # Compare faces
    faces_a = {tuple(sorted(poly.vertices)) for poly in mesh_a.polygons}
    faces_b = {tuple(sorted(poly.vertices)) for poly in mesh_b.polygons}
    if faces_a != faces_b:
        return False

    return True


class WM_OT_batchExportObjs(bpy.types.Operator, ExportHelper):
    """Batch export unique meshes to separate OBJ files and create a JSON scene description"""
    bl_idname = "export_scene.batch_obj"
    bl_label = "Batch export unique OBJ's and create JSON"
    bl_options = {'PRESET', 'UNDO'}

    filename_ext = ".obj"

    filter_glob = StringProperty(
        default="*.obj;*.mtl",
        options={'HIDDEN'},)

    selection_only: BoolProperty(
        name="Selection Only",
        description="Export selected objects only",
        default=True,)

    scale_factor: FloatProperty(
        name="Scale",
        min=0.01, max=1000.0,
        default=1.0)

    axis_forward: EnumProperty(
        name="Forward Axis",
        items=(('X', "X", "Positive X Axis"),
               ('Y', "Y", "Positive Y Axis"),
               ('Z', "Z", "Positive Z Axis"),
               ('NEGATIVE_X', "-X", "Negative X Axis"),
               ('NEGATIVE_Y', "-Y", "Negative Y Axis"),
               ('NEGATIVE_Z', "-Z (Default)", "Negative Z Axis"),),
        default='NEGATIVE_Z')

    axis_up: EnumProperty(
        name="Up Axis",
        items=(('X', "X Up", "Positive X Axis"),
               ('Y', "Y Up (Default)", "Positive Y Axis"),
               ('Z', "Z Up", "Positive Z Axis"),
               ('NEGATIVE_X', "-X Up", "Negative X Axis"),
               ('NEGATIVE_Y', "-Y Up", "Negative Y Axis"),
               ('NEGATIVE_Z', "-Z Up", "Negative Z Axis"),),
        default='Y')

    modifiers_apply: BoolProperty(
        name="Apply Modifiers",
        description="Apply modifiers to exported meshes",
        default=False)

    eval_mode: EnumProperty(
        name="Properties",
        description="Determines properties like object visibility, "
                    "modifiers etc., where they differ for Render and Viewport",
        items=(('DAG_EVAL_VIEWPORT',
                "Viewport (Default)",
                "Export objects as they appear in the viewport"),
               ('DAG_EVAL_RENDER',
                "Render",
                "Export objects as they appear in render"),),
        default='DAG_EVAL_VIEWPORT')

    write_uvs: BoolProperty(
        name="UV Coordinates",
        description="Export the UV coordinates",
        default=True)

    write_normals: BoolProperty(
        name="Normals",
        description="Export per-face normals if the face is flat-shaded, "
                    "per-face-per-loop normals if smooth-shaded",
        default=True)

    write_colors: BoolProperty(
        name="Colors",
        description="Export per-vertex colors",
        default=False)

    triangulate_faces: BoolProperty(
        name="Triangulated Mesh",
        description='All ngons with four or more vertices will be triangulated. '
                    'Meshes in the scene will not be affected. Behaves like '
                    'Triangulate Modifier with ngon-method: "Beauty", quad-method: '
                    '"Shortest Diagonal", min vertices: 4.',
        default=False)

    write_nurbs: BoolProperty(
        name="Curves as NURBS",
        description="Export curves in parametric form instead of exporting as mesh",
        default=False)

    write_materials: BoolProperty(
        name="Export Materials",
        description="Export MTL library. There must be a Principled-BSDF "
                    "node for image textures to be exported to the MTL file.",
        default=True)

    write_pbr: BoolProperty(
        name="PBR Extensions",
        description="Export MTL library using PBR extensions (roughness, metallic, "
                    "sheen, coat, anisotropy, transmission)",
        default=False)

    path_mode: EnumProperty(
        name="Path Mode",
        description="Method used to reference paths",
        items=(('AUTO', "Auto", "Use relative paths with subdirectories only"),
               ('ABSOLUTE', "Absolute", "Always write absolute paths"),
               ('RELATIVE', "Relative", "Write relative paths where possible"),
               ('MATCH', "Match", "OMatch absolute/relative setting with input path"),
               ('STRIP', "Strip", "Write filename only"),
               ('COPY', "Copy", "Copy the file to the destination path"),
               ),
        default='AUTO')

    group_by_object: BoolProperty(
        name="Object Groups",
        description="Append mesh name to object name, separated by a '_'",
        default=False)

    group_by_material: BoolProperty(
        name="Material Groups",
        description="Generate an OBK group for each part of a geometry "
                    "using a different material",
        default=False)

    group_by_vertex: BoolProperty(
        name="Vertex Groups",
        description="Export the name of the vertex group of a face. "
                    "It is approximated by choosing the vertex group "
                    "with the most members amoung the vertices of a face",
        default=False)

    smoothing_groups: BoolProperty(
        name="Smooth Groups",
        description='Every smooth-shaded face is assigned group "1" and '
                    'every flat-shaded face "off".',
        default=False)

    smoothing_group_bitflags: BoolProperty(
        name="Smooth Group Bitflags",
        description="Same as 'Smooth Groups', but generate smooth groups IDs as bitflags "
                    "(produces at most 32 different smooth groups, usually much less)",
        default=False)

    create_json: BoolProperty(
        name="Create JSON Scene",
        description="Generate a JSON file describing the exported scene",
        default=True)

    def execute(self, context):
        folder_path = os.path.dirname(self.filepath)
        viewport_selection = context.selected_objects
        candidates = context.selected_objects if self.selection_only else context.scene.objects

        bpy.ops.object.select_all(action='DESELECT')
        scene_description = {}
        exported_meshes = {}

        for obj in [o for o in candidates if o.type == 'MESH']:
            # Check if this mesh has already been exported
            mesh_data = obj.data
            found_match = None
            for existing_mesh, file_path in exported_meshes.items():
                if are_meshes_identical(mesh_data, existing_mesh):
                    found_match = file_path
                    break

            if not found_match:
                # Export the unique mesh
                file_path = os.path.join(folder_path, f"{obj.data.name}.obj")
                obj.select_set(True)
                bpy.ops.wm.obj_export(
                    filepath=file_path,
                    export_animation=False,
                    forward_axis=self.axis_forward,
                    up_axis=self.axis_up,
                    global_scale=self.scale_factor,
                    apply_modifiers=self.modifiers_apply,
                    export_eval_mode=self.eval_mode,
                    export_selected_objects=True,
                    export_uv=self.write_uvs,
                    export_normals=self.write_normals,
                    export_colors=self.write_colors,
                    export_materials=self.write_materials,
                    export_pbr_extensions=self.write_pbr,
                    path_mode=self.path_mode,
                    export_triangulated_mesh=self.triangulate_faces,
                    export_curves_as_nurbs=self.write_nurbs,
                    export_object_groups=self.group_by_object,
                    export_material_groups=self.group_by_material,
                    export_vertex_groups=self.group_by_vertex,
                    export_smooth_groups=self.smoothing_groups,
                    smooth_group_bitflags=self.smoothing_group_bitflags
                )
                obj.select_set(False)
                exported_meshes[mesh_data] = (f"Objects/{obj.data.name}.obj")
            else:
                file_path = found_match

            # Add object entry to the JSON, referencing the shared OBJ file
            scene_description[obj.name] = {
                "objFilePath": file_path,
                "position": list(obj.location),
                "scale": list(obj.scale),
                "rotation": [obj.rotation_euler.x + 1.570796251296997, obj.rotation_euler.y, obj.rotation_euler.z],
                "u_hasTexture": 1,
                "u_hasShadow": 1,
            }

        if self.create_json:
            scene_file_path = os.path.join(folder_path, "scene.json")
            with open(scene_file_path, 'w') as json_file:
                json.dump(scene_description, json_file, indent=4)

        for obj in viewport_selection:
            obj.select_set(True)

        self.report({'INFO'}, f"Exported {len(exported_meshes)} unique OBJ files and JSON file created")
        return {'FINISHED'}


def menu_func_import(self, context):
    self.layout.operator(WM_OT_batchExportObjs.bl_idname, text="Wavefront Batch (.obj)")


def register():
    bpy.utils.register_class(WM_OT_batchExportObjs)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_import)


def unregister():
    bpy.utils.unregister_class(WM_OT_batchExportObjs)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_import)


if __name__ == "__main__":
    register()
