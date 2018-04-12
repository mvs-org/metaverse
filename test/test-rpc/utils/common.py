import os

def remove_file(file_path):
    if os.path.exists(file_path):
        os.remove(file_path)