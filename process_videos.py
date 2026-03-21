import os
import shutil
import subprocess
import argparse
from pathlib import Path

def process_videos(input_dir, output_dir):
    input_path = Path(input_dir).resolve()
    output_path = Path(output_dir).resolve()

    if not input_path.is_dir():
        print(f"错误：输入目录 '{input_dir}' 不存在。")
        return

    # 获取所有 MP4 文件
    mp4_files = sorted([f for f in input_path.iterdir() if f.suffix.lower() == '.mp4'])
    if not mp4_files:
        print("输入目录中没有找到 MP4 文件。")
        return

    print(f"找到 {len(mp4_files)} 个 MP4 文件，开始处理...")

    for video_file in mp4_files:
        # 寻找下一个可用的数字文件夹名（从1开始递增，直到不存在）
        counter = 1
        while True:
            folder_name = str(counter)
            target_folder = output_path / folder_name
            if not target_folder.exists():
                break
            counter += 1

        # 创建目标文件夹
        target_folder.mkdir(parents=True, exist_ok=False)  # 因为已确保不存在，此处不会触发异常
        print(f"创建文件夹: {target_folder}")

        # 复制原始 MP4 文件到子文件夹
        copied_video = target_folder / video_file.name
        shutil.copy2(video_file, copied_video)
        print(f"复制文件: {video_file.name} -> {copied_video}")

        # 写入原始文件名到 0.txt
        txt_file = target_folder / "0.txt"
        with open(txt_file, 'w', encoding='utf-8') as f:
            f.write(video_file.stem)  # 使用 stem 属性去除扩展名
        print(f"创建文件: {txt_file} (内容: {video_file.stem})")

        # 重命名为 video.mp4
        final_video = target_folder / "video.mp4"
        copied_video.rename(final_video)
        print(f"重命名: {copied_video.name} -> {final_video.name}")

        # 调用 FFmpeg 截图第 3 分钟
        jpg_file = target_folder / "0.jpg"
        cmd = [
            "ffmpeg",
            "-ss", "00:03:00",
            "-i", str(final_video),
            "-vframes", "1",
            "-y",           # 覆盖已存在的输出文件
            str(jpg_file)
        ]
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            print(f"截图成功: {jpg_file}")
        except subprocess.CalledProcessError as e:
            print(f"截图失败 (视频可能不足3分钟): {e.stderr}")
        except FileNotFoundError:
            print("错误：未找到 FFmpeg，请确保已安装并添加到系统 PATH。")
            break

    print("所有视频处理完成。")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="为每个 MP4 文件创建独立文件夹、记录文件名、重命名并截图。")
    parser.add_argument("input_dir", help="包含 MP4 文件的输入文件夹路径")
    parser.add_argument("output_dir", help="输出文件夹路径（将在此创建数字子文件夹）")
    args = parser.parse_args()

    process_videos(args.input_dir, args.output_dir)