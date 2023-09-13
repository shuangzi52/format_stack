文件说明：

- `stack.txt` VSCode、CLion 等编辑器调试过程中得到的堆栈，Frame 类的 format() 方法会读取该文件内容，格式化输出。

- `reformat.txt` 对 Frame 类 format() 方法格式化的结果进行再次格式化，减少层级。
  例如：
  | + - > dispatch_command()
  | + - x > mysql_execute_command()

  减少层级之后变为：

  | > dispatch_command()
  | + > mysql_execute_command()

- `config.ini` 配置文件。
  - `[project_root]` 堆栈中的文件路径需要项目根目录去掉，每一行对应一个项目的根目录，可以配置多行，末尾斜杠（/）可选。
  - `[namespace]` 堆栈中的函数、方法名、参数等前面的表空间，需要把哪些表空间`前缀`去掉，末尾的 :: 可选。