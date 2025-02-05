from moviepy.editor import *

def reduce_movie(input_file, output_file, bitrate="200K"):
    VideoFileClip(input_file).write_videofile(output_file, bitrate=bitrate)

def reduce_movies(input_files, output_directory, bitrate="200K"):
    for file in input_files:
        target = file.replace("\\", '/').split('/')[-1]
        VideoFileClip(file).write_videofile(f"{output_directory}/{target}", bitrate=bitrate)

if __name__ == "__main__":

    A = [1600, 1000, 900, 550]
    B = [2, 6, 4, 3]

    print(sum(map(lambda x, y: x * y), zip(A, B)))
        

