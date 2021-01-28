"""
BSD 2-Clause License
Copyright (c) 2021, timre13
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

from string import Template
import os
import stat

binDir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "build")

fileTemplate = Template("""\
[Desktop Entry]
Version=1.0
Type=Application
Name=LightMusic
GenericName=Music Player
Terminal=false
Exec=sh -c \"cd $binDir; ./lightmusic %F\"
Icon=$binDir/img/icon.png
Comment=LightMusic music player
""")

with open("LightMusic.desktop", "w+") as file:
    file.write(fileTemplate.substitute({"binDir": binDir}))

os.chmod("LightMusic.desktop", stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
