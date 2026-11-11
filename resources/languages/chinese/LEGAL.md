# 法律信息

> 本页面是为方便阅读而提供的翻译。**以英文版本为准**；如有出入，以英文文本（以及可执行文件旁附带的 `LICENSE` 和 `NOTICE` 文件）为准。

## Star Citizen / Cloud Imperium

SCXtool 是一款面向 Star Citizen 的**非官方社区工具**。它并非由 Cloud Imperium Games（CIG）或 Roberts Space Industries（RSI）开发、认可或赞助，也与它们没有任何关联。

**Star Citizen®**、**Roberts Space Industries®** 和 **Cloud Imperium®** 是 Cloud Imperium Rights LLC 和 Cloud Imperium Rights Ltd. 的注册商标。所有 Star Citizen 游戏数据（包括 `Data.p4k` 的内容）均为 Cloud Imperium Rights LLC 的知识产权。

SCXtool 不会再分发任何 CIG 或 RSI 的内容。它从**你自己拥有合法授权的 Star Citizen 安装**中读取文件，并把你自定义的文本写回该安装。游戏中的任何内容都不会通过 SCXtool 离开你的电脑。

## 许可证

SCXtool 衍生自 Smart Citizen（Apache License 2.0，© Osiris DevWorks 及贡献者）和 unp4k（MIT License，© dolkensp 及贡献者）。完整的许可证文本和署名信息随可执行文件旁的 `LICENSE` 和 `NOTICE` 文件一同提供。

第三方库：

- **Qt 6** —— The Qt Company，依据 **GNU LGPL v3** 使用，动态链接。
- **zstd** —— Meta Platforms，**BSD** 许可证。
- **zlib** —— Jean-loup Gailly 和 Mark Adler，**zlib** 许可证。
- **pugixml** —— Arseny Kapoulkine，**MIT** 许可证。

本软件按**“原样”**提供，不附带任何形式的保证或条件。

## 隐私

SCXtool 是一款本地桌面应用程序。它不发送任何遥测、分析数据或崩溃报告，也没有账户。

它只会在以下情况联网：在 GitHub 上检查是否有更新版本的 SCXtool（每次启动时，以及你点击 **检查更新** 时），并在你选择更新时下载它；在你切换到某种语言时下载该语言的社区 `global.ini`；以及在你从 URL 导入 INI 文件时获取该文件。检查更新时，除了应用的版本号之外，不会发送任何关于你或你电脑的信息。你的编辑、设置、备份和缓存都保存在你的电脑上：

- **设置** —— `%APPDATA%\SCXtool\settings.ini`（便携版则为可执行文件旁的 `data\settings.ini`）。
- **你的编辑、备份和字符串缓存** —— 默认位于 `Documents\SCXtool\<频道>\`（可在 配置 标签页中更改）。
- **DataForge 缓存** —— `%LOCALAPPDATA%\SCXtool\<频道>\cache\dataforge\`。
- **日志和崩溃报告** —— `Documents\SCXtool\logs\`。

## 风险自负

SCXtool 会修改本地化文本（CIG 支持社区翻译），以及你在 Game Settings 标签页中选择的游戏选项和按键绑定。如何使用由你自行负责。
