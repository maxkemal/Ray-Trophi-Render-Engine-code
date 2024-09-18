import bpy
import json
import mathutils

def convert_coordinates(vec):
    """Blender'dan alınan koordinatları dönüştür (Z yukarıdan Y yukarıya dönüştür)."""
    return [vec.x, vec.z, vec.y]

def get_camera_data():
    scene = bpy.context.scene
    cam_obj = scene.camera
    if cam_obj is None:
        return None
    
    cam_data = {}
    
    # Pozisyonu al ve koordinatları dönüştür
    lookfrom = convert_coordinates(cam_obj.location)
    
    # Rotasyon matrisinden "lookat" yönünü hesapla
    forward = cam_obj.matrix_world.to_quaternion() @ mathutils.Vector((0.0, 0.0, -1.0))
    lookat = convert_coordinates(cam_obj.location + forward)
    
    # Dik eksen (vup) ve diğer parametreler
    vup = [0, 1, 0]  # Y ekseni yukarı
    vfov = cam_obj.data.angle * (180.0 / 3.14159)  # Radyan cinsinden açıyı dereceye çevir
    aspect_ratio = scene.render.resolution_x / scene.render.resolution_y
    aperture = 0.0  # Blender'da doğrudan bir karşılığı olmayabilir
    focus_dist = 10.0  # Manuel olarak belirlenebilir
    
    # gpu_dof özelliği mevcut mu kontrol edelim
    if hasattr(cam_obj.data, 'gpu_dof') and cam_obj.data.gpu_dof.use_dof:
        blade_count = cam_obj.data.gpu_dof.blades
    else:
        blade_count = 0  # Derinlik efekti kullanılmıyorsa 0
    
    cam_data["lookfrom"] = lookfrom
    cam_data["lookat"] = lookat
    cam_data["vup"] = vup
    cam_data["vfov"] = vfov
    cam_data["aspect_ratio"] = aspect_ratio
    cam_data["aperture"] = aperture
    cam_data["focus_dist"] = focus_dist
    cam_data["blade_count"] = blade_count
    
    return cam_data

def get_lights_data():
    lights = []
    
    for obj in bpy.context.scene.objects:
        if obj.type == 'LIGHT':
            light_data = {}
            light_data['type'] = obj.data.type
            light_data['position'] = convert_coordinates(obj.location)  # Pozisyon dönüşümü
            
            light_data['color'] = list(obj.data.color)
            light_data['power'] = obj.data.energy
            
            if obj.data.type == 'POINT':
                # Sadece pozisyon, renk ve şiddet
                light_data['type'] = 'POINT'
            
            elif obj.data.type == 'SUN':
                # Doğrultu ışığı (Directional Light)
                forward = obj.matrix_world.to_quaternion() @ mathutils.Vector((0.0, 0.0, -1.0))
                light_data['direction'] = convert_coordinates(forward)  # Yön dönüşümü
                light_data['type'] = 'DIRECTIONAL'
            
            elif obj.data.type == 'AREA':
                # Area light için pozisyon, yön, renk, şiddet ve boyutlar
                forward = obj.matrix_world.to_quaternion() @ mathutils.Vector((0.0, 0.0, -1.0))
                light_data['direction'] = convert_coordinates(forward)  # Yön dönüşümü
                light_data['size'] = [obj.data.size, obj.data.size_y] if obj.data.shape == 'RECTANGLE' else [obj.data.size]
                light_data['type'] = 'AREA'
            
            elif obj.data.type == 'SPOT':
                # Spot light için ek özellikler
                light_data['spot_size'] = obj.data.spot_size
                light_data['spot_blend'] = obj.data.spot_blend
                light_data['type'] = 'SPOT'
            
            lights.append(light_data)
    
    return lights

def export_scene_to_json(filepath):
    scene_data = {}
    
    # Kamera bilgilerini al
    camera_data = get_camera_data()
    if camera_data:
        scene_data["camera"] = camera_data
    
    # Işık bilgilerini al
    scene_data["lights"] = get_lights_data()
    
    # JSON dosyasına yaz
    with open(filepath, 'w') as json_file:
        json.dump(scene_data, json_file, indent=4)

# JSON dosyasını kaydet
export_scene_to_json("d:/data/home/file.json")
