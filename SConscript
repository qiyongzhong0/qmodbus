from building import *

cwd = GetCurrentDir()
path = [cwd+'/inc']
src  = Glob('src/*.c')
src += Glob('sample/*.c')
 
group = DefineGroup('modbus', src, depend = ['PKG_USING_QMODBUS'], CPPPATH = path)

Return('group')