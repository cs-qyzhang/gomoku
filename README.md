# gomoku
Alpha-Beta剪枝智能五子棋

使用GTK+框架作为UI，并使用AppImage进行发布

编译使用make命令

若要使用AppImage格式进行发布，使用以下命令：

```shell
wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
chmod a+x appimagetool-x86_64.AppImage
./appimagetool-x86_64.AppImage appimage
```

详见[AppImageKit](https://github.com/AppImage/AppImageKit#appimagetool-usage)
