# KillProcessTcp  
根据进程名杀死网络连接   

该代码用途，对于某些已有的连接，你在开启代理之后，以后的连接并不会断开，   
为了让目标进程立马走代理，我们可能需要手工杀死目标进程的连接，本代码用途   
是于此。   

请用VS编译，具体版本取决于所调用winAPI支持的版本，代码中用到的winAPI不多，   
可自行msdn功能。  

后续改进：当前输入进支持单个进程名，后续将支持多个进程名列表的输入，如一次输入：   
输入要杀死的进程名:a.exe,b.exe,c.exe,d.exe  

Over!
