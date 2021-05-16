#!/usr/bin/env python
'''
fmav_generate_c_library.py
calls fastMavlink generator for C header files
(c) OlliW, OlliW42, www.olliw.eu
'''
import os
import shutil
import re
import sys

#options to set

mavlinkpathtorepository = 'fastmavlink'

mavlinkdialect = "edgetx.xml"

mavlinkoutputdirectory = 'out'

'''
Imports
'''
sys.path.insert(0,mavlinkpathtorepository)

from generator import fmav_gen as mavgen
from generator.modules import fmav_flags as mavflags

'''
Generates the header files and place them in the output directory.
'''

outdir = mavlinkoutputdirectory
xmlfile = mavlinkdialect

print('----------')
print('kill out dir')
try:
    shutil.rmtree(outdir)
except:
    pass    
os.mkdir(outdir)
print('----------')

opts = mavgen.Opts(outdir, parse_flags=mavflags.PARSE_FLAGS_WARNING_ENUM_VALUE_MISSING)
args = [xmlfile]
try:
    mavgen.fmavgen(opts,args)
    print('Successfully Generated Headers', 'Headers generated successfully.')

except Exception as ex:
    exStr = str(ex)
    print('Error Generating Headers','{0!s}'.format(exStr))
    exit()

