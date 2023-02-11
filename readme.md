## Renderflow

// TODO tensorflow gpu support

Installation:

1. Install Python3 (<3.11, because tensorflow support is missing there)
2. 'pip install numpy tensorflow'
3. clone Renderflow to extPlugins
4. get https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-windows-x86_64-2.10.0.zip
5. copy include/tensorflow.dll to RenderFlow/include and to PADrend/build
6. configure and build in release mode
7. enjoy

'export PYTHONFAULTHANDLER=1' for more detailed error messages
