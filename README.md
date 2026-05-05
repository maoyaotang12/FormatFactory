README.md
🎬 FormatFactory 万能格式转换器
一款基于 Qt + FFmpeg + OpenCV + FontForge 开发的全能离线格式转换工具
支持视频、音频、图片、字体四大类文件一键互转，轻量、简洁、无广告、纯本地离线处理，保护用户文件隐私。

---
📌 软件介绍
FormatFactory 万能格式转换器，专为 Linux 平台打造，界面干净清爽，操作简单易用。
不需要复杂命令行，全部图形化操作，普通用户也能轻松完成各类格式转换工作。
✅ 支持转换类型
- 视频转换：MP4、MKV、AVI、MOV、FLV、WebM 等互转
- 音频转换：MP3、FLAC、WAV、AAC、OGG、M4A 等互转
- 图片转换：PNG、JPG、BMP、GIF、WebP 等图片格式互转
- 字体转换：TTF、OTF、WOFF 等字体格式转换处理

---
💡 软件特色
- 🌗 完美深浅色主题自适应，跟随系统自动切换
- 🚀 纯本地离线转换，不上传任何用户文件
- ⚙️ 基于 FFmpeg 核心，转换质量高、速度快
- 🖥️ Qt6 精美界面，布局整洁，操作简单
- 📦 提供 Arch Linux PKGBUILD 一键打包安装
- 📄 开源 MIT 协议，免费使用、自由修改

---
🔧 编译依赖
编译运行需要以下依赖库：
- qt6-base
- qt6-multimedia
- ffmpeg
- opencv
- fontforge
- cmake、gcc、make

---
📦 Arch Linux 安装方法
项目提供完整 PKGBUILD 打包支持：
makepkg -si
安装后可直接在应用菜单搜索 万能格式转换器 打开使用。

---
🛠️ 手动编译构建
cmake -B build
make -C build
make -C build install

---
📜 开源协议
本项目基于 MIT License 开源。
你可以自由使用、修改、分发，保留原始版权声明即可。

---
👨‍💻 作者
maoyaotang
欢迎 Star、Fork、交流学习 ✨感谢豆包