# 编译顺序

libdtkcommon---dtkcore---dtkgui----dtkwidget

编译方法：`fakeroot dpkg-buildpackage`
提示哪里缺依赖就补哪里


编译完成后需要先行安装上一个的dev包

dde-qt5integration

libdframework-dbus2

在以上组件编译完成后可以编译
