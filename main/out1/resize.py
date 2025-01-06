from PIL import Image
import os

def resize_images_in_folder(input_dir, output_dir, target_size=(120, 120)):
    """
    将文件夹中的所有图片缩放到指定尺寸。
    
    :param input_dir: 输入文件夹路径
    :param output_dir: 输出文件夹路径
    :param target_size: 缩放后的图片尺寸 (宽, 高)
    """
    try:
        # 确保输出目录存在
        os.makedirs(output_dir, exist_ok=True)
        
        # 遍历输入目录中的所有图片
        for file_name in os.listdir(input_dir):
            input_path = os.path.join(input_dir, file_name)
            
            # 检查是否为图片文件
            if not file_name.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif')):
                print(f"跳过非图片文件: {file_name}")
                continue
            
            # 打开图片
            img = Image.open(input_path)
            width, height = img.size
            
            # 检查图片大小（可选）
            if width != 240 or height != 240:
                print(f"跳过非 240x240 图片: {file_name}")
                continue
            
            # 缩放图片
            resized_img = img.resize(target_size, Image.ANTIALIAS)
            
            # 保存缩放后的图片
            output_path = os.path.join(output_dir, file_name)
            resized_img.save(output_path)
            print(f"已保存: {output_path}")
    
    except Exception as e:
        print(f"发生错误: {e}")

# 示例调用
if __name__ == "__main__":
    input_folder = "./"  # 输入文件夹路径
    output_folder = "./120_120"  # 输出文件夹路径
    
    resize_images_in_folder(input_folder, output_folder)

