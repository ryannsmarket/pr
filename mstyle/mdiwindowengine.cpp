//////////////////////////////////////////////////////////////////////////////
// oxygenmdiwindowengine.cpp
// stores event filters and maps widgets to timelines for animations
// -------------------
//
// Copyright (c) 2010 Hugo Pereira Da Costa <hugo.pereira@free.fr>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "mdiwindowengine.h"


//____________________________________________________________
bool MdiWindowEngine::registerWidget( QWidget* widget ) {

      if ( !widget ) return false;

      // create new data class
      if ( !data_.contains( widget ) ) data_.insert( widget, new MdiWindowData( this, widget, duration() ), enabled() );

      // connect destruction signal
      connect( widget, SIGNAL( destroyed( QObject* ) ), this, SLOT( unregisterWidget( QObject* ) ), Qt::UniqueConnection );
      return true;

      }
