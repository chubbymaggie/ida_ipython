#What and Why?
This is a plugin to embed an IPython kernel in IDA Pro. The Python ecosystem has amazing libraries (and communities) for scientific computing. IPython itself is great for exploratory data analysis. Using tools such as the IPython notebook make it easy to share code and explanations with rich media. IPython makes using IDAPython and interacting with IDA programmatically really fun and easy.

##Example Uses
##QT Console
You can just use IPython qtconsole for a better interactive python shell for IDA.

![Image of Basic QT Usage](qtbasic.gif)


You can also use the QT console to graph things. This is an example creating a bar chart for the occurrences of each instruction mnemonic in a function (in notepad.exe).

![Image of QT with graph](qtwithgraph.gif)

###Notebooks

Another useful case is using IPython notebooks.

- [Function Entropy](http://nbviewer.ipython.org/github/james91b/ida_ipython/blob/master/notebook/examples/Function%20Entropy.ipynb) - Here is an example where we compute the entropy (using scipy stats module) of each function in notepad.exe and graph the result.

More examples..soon...

#How the plugin works
IDA is predominantly single threaded application, so we cannot safely run the kernel in a separate thread. So instead of using another thread a hook is created on the QT process events function and the "do_one_iteration" method of the ipython kernel is executed each frame.

#Installation
I suggest using the Anaconda distribution of Python as it comes with all the required python libraries pre-built and installed. Alternatively you can install IPython and the dependencies separately.

This plugin should work on all 6.X x86 QT versions of IDA.

##Basic Installation and QTConsole
1. Download and extract the [release](https://github.com/james91b/ida_ipython/releases/tag/0.1)
2. Copy files from `python` and `plugins` directories into your IDA directory.
3. Launch IDA.
4. Start an IPython qtconsole with the kernel instance (outputted in the IDA console) e.g `ipython qtconsole --existing kernel-4264.json`

##Using the Notebook
1. Copy `idc` directory to your IDA directory. (the nothing.idc scipt is used to pass command line parameters to the plugin)
2. Copy IPython profile `notebook\profile_ida` to `%HOME%\.ipython\profile_ida` (All the configuration is default except one line to set the kernel manager in `ipython_notebook_config.py`)
3. Change the IDA_EXE variable to your idaq.exe location then copy `notebook\idakernelmanager.py` to somewhere accessible on your python path e.g `%PYTHONHOME%\Lib\site-packages`
4. Run `ipython notebook --profile ida` at the command line
5. Start a notebook and IDA should start

#How to Build
1. Install cmake
2. At the command line cd to the root directory and run the following
3. `cd build`
4. `cmake -G "Visual Studio 11" -D PYTHON_DIR="<YOUR_PYTHON_DIR>" -D IDA_SDK="<YOUR_IDASDK_LOCATION>"" -D IDA_DIR="<YOUR_IDA_DIRECTORY>" ..`
e.g.
`cmake -G "Visual Studio 11" -D PYTHON_DIR=":\Anaconda" -D IDA_SDK=C:\dev\IDA\idasdks\idasdk64 -D IDA_DIR="C:/Pr
ogram Files (x86)/IDA 6.4" ..`
5. `cmake --build . --config Release`

So far only tested with "Visual Studio 11" compiler.

#To do/Future Ideas
- x64 Support
- More examples
- Create a library for cell/line magic functions specific to IDA
