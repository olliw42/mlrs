#!/usr/bin/env python
'''
*******************************************************
 Copyright (c) MLRS project
 GPL3
 https://www.gnu.org/licenses/gpl-3.0.de.html
 OlliW @ www.olliw.eu
*******************************************************
 version 26.10.2024
*******************************************************
'''
import os
import shutil
import re
import sys
import venv
import pathlib
import subprocess

script_folder = pathlib.Path(__file__).parent.resolve()
venv_folder = os.path.join(script_folder, 'venv')
venv.create(venv_folder, with_pip=True)
python_path = os.path.join(venv_folder, "bin", "python")

def pip_install(package):
    res = subprocess.call([python_path, "-m", "pip",  "install",  package])

pip_install("setuptools")
pip_install("empy==3.3.4")
pip_install("pexpect")
pip_install("dronecan")

mLRSProjectdirectory = os.path.dirname(os.path.abspath(__file__))
mLRSdirectory = os.path.join(mLRSProjectdirectory,'mLRS')

outdir = 'out'


def kill_outdir():
    print('----------')
    print('kill out dir')
    try:
        shutil.rmtree(outdir)
    except:
        pass    
    os.mkdir(outdir)
    print('----------')


def os_system(arg):
    res = os.system(f"{python_path} {arg}")
    if res != 0:
        print('# ERROR (errno =',res,') DONE #')
        os.system("pause")
        exit(1)


def generate_dsdl():
    print('----------------------------------------')
    print(' run dronecan_dsdlc/dronecan_dsdlc.py')
    print('----------------------------------------')
    #os.chdir(os.path.join(mLRSdirectory,'Common','mavlink'))
    
    os_system(
        os.path.join('..','..','modules','dronecan','dronecan_dsdlc','dronecan_dsdlc.py')
        + ' -O ' + os.path.join(outdir)
        + ' ' + os.path.join('..','..','modules','dronecan','DSDL','dronecan')
        + ' ' + os.path.join('..','..','modules','dronecan','DSDL','uavcan')
        #+ ' ' + os.path.join('..','..','modules','dronecan','DSDL','com')
        #+ ' ' + os.path.join('..','..','modules','dronecan','DSDL','ardupilot')
    )


desired_dsdl_list = [
    'dronecan.sensors.rc.RCInput',
    
    'uavcan.protocol.NodeStatus',
    
    'uavcan.protocol.GetNodeInfo',
    'uavcan.protocol.GetNodeInfo_req',
    'uavcan.protocol.GetNodeInfo_res',
    'uavcan.protocol.HardwareVersion',
    'uavcan.protocol.SoftwareVersion',
    
    'uavcan.protocol.dynamic_node_id.Allocation',
    
    'uavcan.tunnel.Targetted',
    'uavcan.tunnel.Protocol',

    'dronecan.protocol.FlexDebug',
]    


def remove_dsdl():
    print('----------------------------------------')
    print(' remove unneeded dsdl')
    print('----------------------------------------')

    for path, subdirs, files in os.walk(os.path.join(outdir,'include')):
        for file in files:
            if file[:-2] not in desired_dsdl_list:
                os.remove(os.path.join(outdir,'include',file))
            else:
                print('KEEP',file)

    for path, subdirs, files in os.walk(os.path.join(outdir,'src')):
        for file in files:
            if file[:-2] not in desired_dsdl_list:
                os.remove(os.path.join(outdir,'src',file))
            else:
                print('KEEP',file)


def call_replace_h(filepath, filename):
    F = open(os.path.join(filepath,filename), mode='r')
    content = F.read()
    F.close()

#    content1 = re.sub(r'#include <canard.h>', r'#include "../../libcanard/canard.h" //XX#include <canard.h>', content)
    content1 = re.sub(r'#include <canard.h>', r'#include "../../../../modules/stm32-dronecan-lib/libcanard/canard.h" //XX#include <canard.h>', content)
    content2 = re.sub(r'#include <uavcan.([a-zA-Z\._]+?).h>', r'#include "uavcan.\1.h" //XX#include <uavcan.\1.h>', content1)

    F = open(os.path.join(filepath,filename), mode='w')
    F.write(content2)
    F.close()
    

def call_replace_c(filepath, filename):
    F = open(os.path.join(filepath,filename), mode='r')
    content = F.read()
    F.close()

    content1 = re.sub(r'#include <uavcan.([a-zA-Z\._]+?).h>', r'#include "../include/uavcan.\1.h" //XX#include <uavcan.\1.h>', content)
    content2 = re.sub(r'#include <dronecan.([a-zA-Z\._]+?).h>', r'#include "../include/dronecan.\1.h" //XX#include <dronecan.\1.h>', content1)

    F = open(os.path.join(filepath,filename), mode='w')
    F.write(content2)
    F.close()


def correct_dsdl():
    print('----------------------------------------')
    print(' correct dsdl')
    print('----------------------------------------')

    for path, subdirs, files in os.walk(os.path.join(outdir,'include')):
        for file in files:
            print(file)
            call_replace_h(os.path.join(outdir,'include'), file)
            
    for path, subdirs, files in os.walk(os.path.join(outdir,'src')):
        for file in files:
            print(file)
            call_replace_c(os.path.join(outdir,'src'), file)


#-- here we go
if __name__ == "__main__":
    cmdline_nopause = False

    cmd_pos = -1
    for cmd in sys.argv:
        cmd_pos += 1
        if cmd == '--nopause' or cmd == '-np':
            cmdline_nopause = True


    kill_outdir()
    generate_dsdl()
    remove_dsdl()
    correct_dsdl()


    print('Done')
    if not cmdline_nopause:
        os.system("pause")

