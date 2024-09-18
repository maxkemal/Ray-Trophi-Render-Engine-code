import bpy
import json
import os
import shutil
import mathutils

bl_info = {
    "name": "GLTF ve JSON Dışa Aktarıcı",
    "blender": (3, 6, 5),
    "category": "Import-Export",
    "description": "GLTF ve JSON formatında dışa aktarma yapar.",
    "author": "Kemal Demirtaş",
    "version": (1, 0),
    "support": "COMMUNITY",
    "license": "GPL-3.0",
}

class ExportGLTFAndJSONOperator(bpy.types.Operator):
    bl_idname = "export.gltf_and_json"
    bl_label = "GLTF ve JSON Dışa Aktar"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        gltf_export_path = context.scene.gltf_export_path
        gltf_dir = os.path.dirname(bpy.path.abspath(gltf_export_path))  # GLTF dosyasının dizini
        textures_dir = os.path.join(gltf_dir, "textures")  # Texture'ların saklanacağı klasör

        # GLTF olarak sahneyi Separate modunda (ayrı .gltf, .bin ve texture dosyaları) dışa aktarma
        bpy.ops.export_scene.gltf(filepath=gltf_export_path, export_format='GLTF_SEPARATE')

        # Blender'daki tüm image (texture) dosyalarını al
        textures = bpy.data.images

        # Textures klasörünü oluştur
        if not os.path.exists(textures_dir):
            os.makedirs(textures_dir)

        # Texture dosyalarını kopyalamak için bir set oluştur
        unique_textures = set()

        # Tüm texture dosyalarını textures klasörüne kopyala
        for texture in textures:
            if texture.filepath:
                texture_path = bpy.path.abspath(texture.filepath)
                if os.path.exists(texture_path):
                    # Hedef dizin içindeki dosya adı
                    texture_name = os.path.basename(texture_path)
                    
                    # Eğer texture adı daha önce kullanıldıysa geç
                    if texture_name not in unique_textures:
                        unique_textures.add(texture_name)
                        target_path = os.path.join(textures_dir, texture_name)
                        
                        # Texture'ı textures klasörüne kopyala
                        shutil.copy(texture_path, target_path)
                        self.report({'INFO'}, f"Texture copied: {texture_path} to {target_path}")

        # JSON dosyasını GLTF dosyasıyla aynı konuma kaydedeceğiz
        json_export_path = os.path.splitext(gltf_export_path)[0] + "_scene_data.json"

        # JSON verilerini saklayacağımız yapı
        scene_data = {
            "lights": [],
            "cameras": []
        }

        # Işıklar
        for obj in bpy.data.objects:
            if obj.type == 'LIGHT':
                light_data = {
                    "type": obj.data.type,  # 'POINT', 'SUN', 'SPOT', 'AREA'
                    "color": list(obj.data.color),  # RGB color
                    "energy": obj.data.energy  # Intensity
                }
                
                # Işık bir yön belirtirse, ona uygun vektörü ekliyoruz
                if obj.data.type in ['SPOT', 'SUN']:
                    light_data["direction"] = list(obj.matrix_world.to_quaternion() @ mathutils.Vector((0.0, 0.0, -1.0)))
                
                # Işık bir konuma sahipse, konumu ekliyoruz
                light_data["location"] = list(obj.location)
                
                scene_data["lights"].append(light_data)

        # Kameralar
        for obj in bpy.data.objects:
            if obj.type == 'CAMERA':
                cam = obj.data
                
                # Kameranın bakış noktası ve yukarı vektörü
                lookfrom = obj.matrix_world.translation
                lookat = obj.matrix_world.translation + obj.matrix_world.to_quaternion() @ mathutils.Vector((0.0, 0.0, -1.0))
                vup = obj.matrix_world.to_quaternion() @ mathutils.Vector((0.0, 1.0, 0.0))

                # Kameranın diğer parametreleri
                vfov = cam.angle_y  # Dikey görüş alanı
                aspect = cam.sensor_width / cam.sensor_height if cam.sensor_fit == 'AUTO' else (cam.sensor_width / cam.sensor_height if cam.sensor_fit == 'HORIZONTAL' else cam.sensor_height / cam.sensor_width)
                aperture = cam.dof.aperture_fstop  # Diyafram açıklığı
                focus_dist = cam.dof.focus_distance if cam.dof.use_dof else 0.0
                blade_count = cam.dof.aperture_blades if cam.dof.use_dof else 0  # Diyafram yaprak sayısı
                
                # Kamerayı Camera sınıfınıza uygun formatta JSON dosyasına ekliyoruz
                camera_data = {
                    "lookfrom": list(lookfrom),
                    "lookat": list(lookat),
                    "vup": list(vup),
                    "vfov": vfov,
                    "aspect": aspect,
                    "aperture": aperture,
                    "focus_dist": focus_dist,
                    "blade_count": blade_count
                }
                
                scene_data["cameras"].append(camera_data)

        # JSON dosyasını yazma
        with open(json_export_path, 'w') as json_file:
            json.dump(scene_data, json_file, indent=4)

        self.report({'INFO'}, f"GLTF ve JSON dosyası başarıyla dışa aktarıldı: {gltf_export_path}, {json_export_path}")
        return {'FINISHED'}

class ExportGLTFAndJSONPanel(bpy.types.Panel):
    bl_label = "GLTF ve JSON Dışa Aktarıcı"
    bl_idname = "PT_ExportGLTFAndJSON"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'Export'

    def draw(self, context):
        layout = self.layout
        scene = context.scene
        layout.prop(scene, "gltf_export_path")
        layout.operator("export.gltf_and_json", text="Export GLTF and JSON")

def register():
    bpy.utils.register_class(ExportGLTFAndJSONOperator)
    bpy.utils.register_class(ExportGLTFAndJSONPanel)
    bpy.types.Scene.gltf_export_path = bpy.props.StringProperty(
        name="GLTF Export Path",
        description="Path to save the exported GLTF file",
        default="//scene.gltf",
        subtype='FILE_PATH'
    )

def unregister():
    if hasattr(bpy.types, "ExportGLTFAndJSONOperator"):
        bpy.utils.unregister_class(ExportGLTFAndJSONOperator)
    if hasattr(bpy.types, "ExportGLTFAndJSONPanel"):
        bpy.utils.unregister_class(ExportGLTFAndJSONPanel)
    if hasattr(bpy.types.Scene, "gltf_export_path"):
        del bpy.types.Scene.gltf_export_path

if __name__ == "__main__":
    register()
