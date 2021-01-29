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

#include "AboutWindow.h"
#include "version.h"
#include "license.h"
#include <iostream>
#include <FL/Enumerations.H>

AboutWindow::AboutWindow()
    : Fl_Double_Window(700, 500, "About LightMusic")
{
    m_titleLabel = new Fl_Button{0, 10, 700, 40};
    m_titleLabel->copy_label("LightMusic");
    m_titleLabel->box(FL_NO_BOX);
    m_titleLabel->down_box(FL_NO_BOX);
    m_titleLabel->labelsize(30);
    m_titleLabel->align(FL_ALIGN_CENTER);

    m_versionLabel = new Fl_Button{0, 50, 700, 30};
    m_versionLabel->box(FL_NO_BOX);
    m_versionLabel->down_box(FL_NO_BOX);
    m_versionLabel->copy_label(VERSION_STR);
    m_versionLabel->labelsize(20);
    m_versionLabel->align(FL_ALIGN_CENTER);

    m_iconImage = new Fl_PNG_Image{"img/icon.png"};
    if (m_iconImage->fail())
    {
        std::cerr << "Failed to load icon" << '\n';
    }
    else
    {
        m_iconLabel = new Fl_Button{0, 0, 128, 128};
        m_iconLabel->image(m_iconImage);
    }

    m_licenseLabelBuffer = new Fl_Text_Buffer{};
    m_licenseLabel = new Fl_Text_Display{0, 128, 700, 372};
    m_licenseLabel->textsize(12);
    m_licenseLabel->buffer(m_licenseLabelBuffer);
    m_licenseLabelBuffer->text(LICENSE_STR);
}

AboutWindow::~AboutWindow()
{
    delete m_titleLabel;
    delete m_versionLabel;
    delete m_iconImage;
    delete m_iconLabel;
    delete m_licenseLabel;
    delete m_licenseLabelBuffer;
}
