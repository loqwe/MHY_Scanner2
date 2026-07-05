# MHY_Scanner2

MHY_Scanner2 是基于 [DSVVA/MHY_Scanner](https://github.com/DSVVA/MHY_Scanner) 二次修改的米哈游扫码工具。

本项目为免费开源项目，仅用于学习和研究，禁止商业化用途。请勿提交或公开自己的 Cookie、SToken、MID、UID 等账号敏感信息。

## 二改说明

本仓库在原项目基础上重点修复新版米哈游扫码登录、官服抢码确认、直播间抢码和账号状态检测相关问题，并补充了运行日志，方便定位扫码失败原因。

## 新增功能

- 新版 passport 添加账号扫码登录流程。
- 官服抢码支持 Panda 游戏二维码两阶段登录：先解析游戏二维码，再换取 passport 二维码并确认登录。
- 启动后自动自检账号状态，在账号表显示 `检测中`、`有效`、`失效`、`UID异常`。
- 直播间输入支持 B 站完整链接和纯房间号。
- 直播间 ID 自动保存输入历史，无需保存按钮，最近保留 5 条记录。
- 增加链路日志输出，默认写入 `Config/scanner.log`，便于排查扫码、抢码、直播流解析问题。

## 修复和优化

- 修复 SToken v2 校验失败问题，改用新版账号信息校验接口。
- 修复游戏二维码直接按 passport 二维码处理导致 `missing qr params` 的问题。
- 修复登录确认接口 `token_types` 参数格式错误导致 `retcode=-502` 的问题。
- 二维码参数解析从固定字符串偏移改为 query 参数解析，兼容不同二维码 URL 形态。
- 优化 B 站直播流请求头和房间号解析逻辑。
- 增加直播流初始化超时和异常日志，避免失败时界面长期无反馈。
- 移除手动保存直播间按钮，输入、点击监视和关闭窗口时自动保存。

## 功能和特性

- 从屏幕自动获取二维码登录，适用于大部分登录情景。
- 从直播流获取二维码登录，适用于抢码登录情景。
- 可选启动后自动开始识别屏幕和识别完成后自动退出。
- 表格化管理多账号，方便切换游戏账号和查看账号状态。

## 目前可用的平台

| 崩坏3 | 原神 | 星穹铁道 | 绝区零 |
| :---: | :--: | :------: | :----: |
| 官服 | 官服 | 官服 | 官服 |
| BiliBili |  |  |  |

## 使用说明

运行 `MHY_Scanner.exe`。

点击菜单栏 **账号管理 -> 添加账号**，使用扫码登录添加账号。

双击账号对应的备注单元格可以添加自定义备注。

登录后点击 **监视屏幕**，即可自动识别任意显示在屏幕上的二维码并自动登录。

选择直播平台，在当前直播间输入框输入直播间 ID 或 B 站直播间完整链接，点击 **监视直播间**，即可自动识别该直播间显示的二维码并自动登录。

正在执行任务的按钮会高亮显示，再次点击会停止。

## 编译

请参考 CI/CD 工作流，或使用 Visual Studio 2022 + CMake 构建。

运行前需要安装 [Visual C++ 运行时库](https://aka.ms/vs/17/release/vc_redist.x64.exe)。

## 相关项目

- [DSVVA/MHY_Scanner](https://github.com/DSVVA/MHY_Scanner) - 本项目二改来源。
- [KuRo_Scanner](https://github.com/Theresa-0328/KuRo_Scanner) - 鸣潮扫码器。

## 参考和感谢

- [HonkaiScanner](https://github.com/HonkaiScanner)
- [BililiveRecorder/BililiveRecorder](https://github.com/BililiveRecorder/BililiveRecorder)
- [DGP-Studio/Snap.Hutao](https://github.com/DGP-Studio/Snap.Hutao)
