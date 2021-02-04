/*
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
*/

#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_PNG_Image.H>

/*
 * An about window for LightMusic.
 * Contains the application name, version, a short description and the license.
 */
class AboutWindow : public Fl_Double_Window
{
private:
    Fl_Button *m_titleLabel{};

    Fl_Button *m_versionLabel{};

    Fl_PNG_Image *m_iconImage{};
    Fl_Button *m_iconLabel{};

    Fl_Button *m_webUrlLabel{};

    Fl_Text_Display *m_licenseLabel{};
    Fl_Text_Buffer *m_licenseLabelBuffer{};

    static void s_webUrlLabelCallback(Fl_Widget*, void *t)
    {
        static_cast<AboutWindow*>(t)->webUrlLabelCallback();
    }
    void webUrlLabelCallback();

public:
    AboutWindow();

    AboutWindow(const AboutWindow &other) = delete;
    AboutWindow(AboutWindow &&other) = delete;
    AboutWindow& operator=(const AboutWindow &other) = delete;
    AboutWindow& operator=(AboutWindow &&other) = delete;

    ~AboutWindow();
};

