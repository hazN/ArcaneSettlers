import os
from PIL import Image

source_folder = os.path.dirname(os.path.abspath(__file__))
destination_folder = source_folder

for file in os.listdir(source_folder):
    if file.endswith('.png'):
        img = Image.open(os.path.join(source_folder, file))
        img = img.convert('RGB')
        img = img.transpose(Image.FLIP_TOP_BOTTOM)
        bmp_filename = os.path.splitext(file)[0] + '.bmp'
        img.save(os.path.join(destination_folder, bmp_filename), 'BMP')
        print(f'Converted {file} to {bmp_filename}')

print('Conversion completed.')
