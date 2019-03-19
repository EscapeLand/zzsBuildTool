# zzsBuildTool

HIT SC Lab (kidding) build tool, based JDK. <br>
NOTICE: If you DO NOT know what I'm saying, please look for another bulid tool. This is __NOT__ for unified-use!!!

> 2019-3-19: 1.0.1 发布，点击[这里](https://github.com/EscapeLand/zzsBuildTool/releases/tag/1.0.1)查看

## 使用

放在Labx-11x030xxxx目录里即可；墙裂建议用这东西build之前 __先调整目录结构__ 否则程序可能有未定义的行为。。。

#### Usage

BuildTool _[options]_ _[binary directory]_
    
* -b (path): set your binary directory, default dir is __bin__
* -d: multi-thread enabled (by default)
* -s: silent mode ~~(maybe more efficency)~~
* -t: multi-thread disabled (take the __lastest__ when with -d)
* -v: verbose mode, for trace and debug (take the __lastest__ when with -s)
* --version: print version information

~~当然了，直接双击也好使，全部默认就完了(￣▽￣)"~~

#### Note

* 把依赖放到 __lib__ 文件夹下
* 只扫描 __src__ 下的二进制文件，__test__ 文件夹下的不会打包
* 打包之前请先编译，没有二进制文件难道要我给你`javac`一下？？？
* 确保二进制和源码之间包名的对应关系，比如 <br>源文件夹下是 
  `com/hhh/emmm.java`，那么输出文件夹下就是 `com/hhh/emmm.class` <br>
  我没法保证不这么做会发生什么，反正 ~~正常的~~ 人和IDE都是这么干的
* ~~如果你有问题请先检查一下上面几条而不是骚扰我的QQ~~
* ~~如果你因为用这东西 bulid 得不对得了零蛋，跟我没关系~(￣▽￣)~*~~

## Credit

~~To be Continued...~~