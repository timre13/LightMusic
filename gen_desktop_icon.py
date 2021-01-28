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
