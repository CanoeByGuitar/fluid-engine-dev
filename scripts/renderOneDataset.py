#  [under bin dir]python ../scripts/render_manual_tests_output.py

import fnmatch
import matplotlib.animation as animation
import matplotlib.pyplot as plt
import numpy as np
import os
import platform
import re
import utils

INPUT_ARRAY_FORMAT = '.npy'
OUTPUT_BITMAT_FORMAT = '.png'
OUTPUT_VECTOR_FORMAT = '.pdf'
OUTPUT_MOVIE_FORMAT = '.mp4'

X_TAG = 'x'
Y_TAG = 'y'
Z_TAG = 'z'
ISO_TAG = 'iso'

video_writer = 'ffmpeg'
video_extra_args = ['-vcodec', 'libx264', '-pix_fmt', 'yuv420p']

def remove_ext(filename):
    return filename[:-len(INPUT_ARRAY_FORMAT)]


def parse_tags(filename):
    tokens = remove_ext(filename).split('#')
    if len(tokens) > 1:
        return tokens[1].split(',')
    else:
        return []


def get_output_bitmap_filename(filename):
    return remove_ext(filename) + OUTPUT_BITMAT_FORMAT


def get_output_movie_filename(filename):
    return remove_ext(filename.replace(',0000', '')) + OUTPUT_MOVIE_FORMAT


def is_animation(tags):
    for tag in tags:
        if fnmatch.filter([tag], '[0-9][0-9][0-9][0-9]'):
            return True
    return False


def render_grid2(filename, frame_rate=60):
    if is_animation(parse_tags(filename)):
        dirname = os.path.dirname(filename)
        basename = os.path.basename(filename).replace('0000', '[0-9][0-9][0-9][0-9]')
        seq = utils.get_all_files(dirname, [basename])
        seq.sort()
        if len(seq) == 0:
            return
        grid_data = np.load(seq[0])
        grid_data = np.flipud(grid_data)
        fig, ax = plt.subplots()
        im = ax.imshow(grid_data, cmap="Blues", interpolation='nearest')
        if ISO_TAG in parse_tags(filename):
            plt.contour(grid_data)

        def update_image(i):
            grid_data = np.load(seq[i])
            grid_data = np.flipud(grid_data)
            im.set_array(grid_data)
            if ISO_TAG in parse_tags(filename):
                plt.contour(grid_data)
            return im

        output_filename = get_output_movie_filename('result')
        anim = animation.FuncAnimation(fig, update_image, frames=len(seq), interval=60, blit=False)
        anim.save(output_filename, fps=frame_rate, bitrate=5000, writer=video_writer, extra_args=video_extra_args)
        plt.close(fig)
        print ('Rendered <%s>' % output_filename)
    else:
        tags = parse_tags(filename)
        output_filename = get_output_bitmap_filename(filename)
        if X_TAG in tags:
            grid_data_u = np.load(filename)
            grid_data_v = np.load(filename.replace(X_TAG, Y_TAG))
            render_still_vector_grid2(filename, filename.replace(X_TAG, Y_TAG), output_filename)


def render_still_vector_grid2(filename_x, filename_y, output_filename, **kwargs):
    has_xtick = True
    has_ytick = True
    if 'has_xtick' in kwargs:
        has_xtick = kwargs['has_xtick']
    if 'has_ytick' in kwargs:
        has_ytick = kwargs['has_ytick']
    grid_data_u = np.load(filename_x)
    grid_data_v = np.load(filename_y)
    nx = len(grid_data_u[0])
    ny = len(grid_data_u)
    X, Y = np.meshgrid(np.arange(0, 1, 1.0 / nx), np.arange(0, float(ny) / nx, 1.0 / nx))
    U = grid_data_u
    V = grid_data_v
    fig, ax = plt.subplots()
    if not has_xtick:
        ax.set_xticks(())
        ax.set_xticklabels(())
    if not has_ytick:
        ax.set_yticks(())
        ax.set_yticklabels(())
    ax.set_aspect('equal')
    plt.quiver(X, Y, U, V)
    plt.savefig(output_filename)
    plt.close(fig)
    print('Rendered <%s>' % output_filename)


if __name__ == '__main__':
    filenames = utils.get_all_files('/Users/wangchenhui/github/fluid-engine-dev/bin/test_datasets/case_no_obstacle_0.10x0.50/phi',
                                    ['*' + INPUT_ARRAY_FORMAT])
    filenames.sort()
    for filename in filenames:
        try:
            # TODO: grid2 only now
            render_grid2(filename, 60)
        except Exception as e:
            print ('Failed to render', filename)
            print ('Why?')
            print (e)
