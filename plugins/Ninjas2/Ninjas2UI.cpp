/*
 * Ninjas 2
 *
 * Copyright (C) 2018 Clearly Broken Software
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "DistrhoUI.hpp"
#include "Ninjas2UI.hpp"
#include <iostream>
#include <string>
#include "DistrhoPluginInfo.h"

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------
NinjasUI::NinjasUI()
  : UI ( 1200, 600 )
{
  // init parameters
  samplerate = getSampleRate();
  // adsr
  std::fill_n ( p_Attack, 128, 0.05f );
  std::fill_n ( p_Decay, 128, 0.05f );
  std::fill_n ( p_Sustain, 128, 1.0f );
  std::fill_n ( p_Release, 128, 0.05f );

  // play modes
  std::fill_n ( p_OneShotFwd, 128, 1.0f );
  std::fill_n ( p_OneShotRev, 128, 0.0f );
  std::fill_n ( p_LoopFwd, 128, 0.0f );
  std::fill_n ( p_LoopRev, 128, 0.0f );

  // slices
  enum_slicemode = RAW;
  slicemethod = false; // TODO remove the bool and use the enum
  currentSlice = 0;
  slices = 1;
  std::fill_n ( a_slices,128, Slice() );

  // sample
  sampleSize = 0;
  sampleChannels = 1;
  sample_is_loaded = false;

  // waveform
  waveView.start = 0;
  waveView.end = 0;
  waveView.zoom = 1.0f;
  display.setSize ( 1140,350 );
  display.setPos ( 30,50 );

  // mouse
  mouseDragging = false;

  // knobs
  const Size<uint> knobSize = Size<uint> ( 80, 80 );
  const Color ninjasColor = Color ( 222,205,135,255 );

  fKnobSlices = new VolumeKnob ( this, knobSize );
  fKnobSlices->setId ( paramNumberOfSlices );
  fKnobSlices->setRange ( 1.0f, 128.0f );
  fKnobSlices->setColor ( ninjasColor );
  fKnobSlices->setStep ( 1.0f );
  fKnobSlices->setCallback ( this );

  fKnobAttack = new VolumeKnob ( this, knobSize );
  fKnobAttack->setId ( paramAttack );
  fKnobAttack->setRange ( 0.05f, 1.0f );
  fKnobAttack->setColor ( ninjasColor );
  fKnobAttack->setCallback ( this );

  fKnobDecay = new VolumeKnob ( this, knobSize );
  fKnobDecay->setId ( paramDecay );
  fKnobDecay->setRange ( 0.05f, 1.0f );
  fKnobDecay->setColor ( ninjasColor );
  fKnobDecay->setCallback ( this );

  fKnobSustain = new VolumeKnob ( this, knobSize );
  fKnobSustain->setId ( paramSustain );
  fKnobSustain->setRange ( 0.0f, 1.0f );
  fKnobSustain->setColor ( ninjasColor );
  fKnobSustain->setCallback ( this );

  fKnobRelease = new VolumeKnob ( this, knobSize );
  fKnobRelease->setId ( paramRelease );
  fKnobRelease->setRange ( 0.05f, 1.0f );
  fKnobRelease->setColor ( ninjasColor );
  fKnobRelease->setCallback ( this );

  //slider TODO make tripolar switch | RAW | ONSETS | MANUAL |

  fSliceModeSlider = new BipolarModeSwitch ( this, Size<uint> ( 16, 34 ) );
  fSliceModeSlider->setCallback ( this );
  fSliceModeSlider->setId ( paramSliceMode );

  fLabelsBoxSliceModeSlider = new GlowingLabelsBox ( this, Size<uint> ( 58, 42 ) );
  fLabelsBoxSliceModeSlider->setLabels ( {"RAW", "ONSETS"} );
  fLabelsBoxLoadSample = new GlowingLabelsBox ( this, Size<uint> ( 90, 70 ) );
  fLabelsBoxLoadSample->setLabels ( {"Load Sample" } );

  // switches

  // play modes

  const Size<uint> switchSize = Size<uint> ( 50, 50 );
  const Size<uint> gridSize = Size<uint> ( 25, 25 );


  fSwitchFwd = new RemoveDCSwitch ( this, switchSize );
  fSwitchFwd->setId ( paramOneShotFwd );
  fSwitchFwd->setCallback ( this );

  fSwitchRev = new RemoveDCSwitch ( this, switchSize );
  fSwitchRev->setId ( paramOneShotRev );
  fSwitchRev->setCallback ( this );

  fSwitchLoopFwd = new RemoveDCSwitch ( this, switchSize );
  fSwitchLoopFwd->setId ( paramLoopFwd );
  fSwitchLoopFwd->setCallback ( this );

  fSwitchLoopRev = new RemoveDCSwitch ( this, switchSize );
  fSwitchLoopRev->setId ( paramLoopRev );
  fSwitchLoopRev->setCallback ( this );

  // sample load button

  fSwitchLoadSample = new RemoveDCSwitch ( this, switchSize );
  fSwitchLoadSample->setId ( paramLoadSample );
  fSwitchLoadSample->setCallback ( this );

  // grid

  for ( int i = paramSwitch01, j = 0 ; i <= paramSwitch16; ++i , ++j )
    {
      fGrid[j] = new RemoveDCSwitch ( this, gridSize );
      fGrid[j]->setId ( i );
      fGrid[j]->setCallback ( this );
    }

  positionWidgets();
  // text
  loadSharedResources();
  fNanoFont = findFont ( NANOVG_DEJAVU_SANS_TTF );
}

void NinjasUI::positionWidgets()
{
  //const float width = getWidth();
  //const float height = getHeight();

  fKnobSlices->setAbsolutePos ( 200, 440 );
  fKnobAttack->setAbsolutePos ( 660, 465 );
  fKnobDecay->setAbsolutePos ( 760, 465 );
  fKnobSustain->setAbsolutePos ( 860, 465 );
  fKnobRelease->setAbsolutePos ( 960, 465 );

  fSliceModeSlider->setAbsolutePos ( 200, 540 );
  fLabelsBoxSliceModeSlider->setAbsolutePos ( 230, 540 );

  fSwitchFwd->setAbsolutePos ( 490, 450 );
  fSwitchRev->setAbsolutePos ( 560, 450 );
  fSwitchLoopFwd->setAbsolutePos ( 490, 510 );
  fSwitchLoopRev->setAbsolutePos ( 560, 510 );
  fSwitchLoadSample->setAbsolutePos ( 50, 495 );
  fLabelsBoxLoadSample->setAbsolutePos ( 30, 480 );

  // set coordinates for grid

  // x = 980, y = 90

  for ( int y = 0 ; y < 4 ; ++y )
    {
      for ( int x = 0 ; x < 4 ; ++x )
        {
          int index = y * 4 + x;
          fGrid[index]->setAbsolutePos ( 330+x*30,450+y*30 );
        } // for x
    } // for y
}

/**
   A parameter has changed on the plugin side.
   This is called by the host to inform the UI about parameter changes.
 */
void NinjasUI::parameterChanged ( uint32_t index, float value )
{
  switch ( index )
    {
    case paramNumberOfSlices:
      fKnobSlices->setValue ( value );
      slices = value ;
      createSlicesRaw ();
      break;
      // Play Modes
    case paramOneShotFwd:
      fSwitchFwd->setDown ( value > 0.5f );
      p_OneShotFwd[currentSlice] = value > 0.5f;
      break;
    case paramOneShotRev:
      fSwitchRev->setDown ( value > 0.5f );
      p_OneShotRev[currentSlice] = value > 0.5f;
      break;
    case paramLoopFwd:
      fSwitchLoopFwd->setDown ( value > 0.5f );
      p_LoopFwd[currentSlice] = value > 0.5f;
      break;
    case paramLoopRev:
      fSwitchLoopRev->setDown ( value > 0.5f );
      p_LoopRev[currentSlice] = value > 0.5f;
      break;
      // ADSR
    case paramAttack:
      fKnobAttack->setValue ( value );
      p_Attack[currentSlice] = value;
      break;
    case paramDecay:
      fKnobDecay->setValue ( value );
      p_Decay[currentSlice] = value;
      break;
    case paramSustain:
      fKnobSustain->setValue ( value );
      p_Sustain[currentSlice] = value;
      break;
    case paramRelease:
      fKnobRelease->setValue ( value );
      p_Release[currentSlice] = value;
      break;
    case paramLoadSample:
      if ( sample_is_loaded )
        {
          fSwitchLoadSample->setDown ( true );
        }
      else
        {
          fSwitchLoadSample->setDown ( false );
        }
      break;

    case paramSliceMode:
      fSliceModeSlider->setDown ( value > 0.5f );
      break;
    }

  // selector grid

  if ( index >= paramSwitch01 && index <= paramSwitch16 )
    {
      int slice = index - paramSwitch01;
      fGrid[slice]->setDown ( value > 0.5f );
      if ( fGrid[slice]->isDown() )
        {
          currentSlice = slice;
          recallSliceSettings ( slice );
        }
    }

  repaint();
}

void NinjasUI::stateChanged ( const char* key, const char* value )
{
  if ( std::strcmp ( key, "filepath" ) == 0 )
    {
      loadSample ( String ( value ) );
    }
}


void NinjasUI::uiFileBrowserSelected ( const char* filename )
{
  if ( filename != nullptr )
    {
      // if a file was selected, tell DSP
      directory = dirnameOf ( filename );
      setState ( "filepath", filename );
      loadSample ( String ( filename ) );
    }
}
/* ----------------------------------------------------------------------------------------------------------
 * Widget Callbacks
 *----------------------------------------------------------------------------------------------------------*/


void NinjasUI::nanoKnobValueChanged ( NanoKnob* knob, const float value )
{
  int KnobID = knob->getId();

  setParameterValue ( KnobID,value );

  switch ( KnobID )
    {
    case paramNumberOfSlices:
      slices = value;
      if ( !slicemethod )
        {
          createSlicesRaw ();
        }
      else
        {
          createSlicesOnsets ();
        }
      break;

    case paramAttack:
      p_Attack[currentSlice]=value;
      break;
    case paramDecay:
      p_Decay[currentSlice]=value;
      break;
    case  paramSustain:
      p_Sustain[currentSlice]=value;
      break;
    case paramRelease:
      p_Release[currentSlice]=value;
      break;
    default:
      setParameterValue ( KnobID,value );

    }

  repaint();
}

void NinjasUI::nanoSwitchClicked ( NanoSwitch* nanoSwitch )
{
  const float value = nanoSwitch->isDown() ? 1.0f : 0.0f;
  const uint buttonId = nanoSwitch->getId();

  switch ( buttonId )
    {
    case paramOneShotFwd:
    {
      p_OneShotFwd[currentSlice] = 1;
      p_OneShotRev[currentSlice] = 0;
      p_LoopFwd[currentSlice]    = 0;
      p_LoopRev[currentSlice]    = 0;

      editParameter ( paramOneShotFwd, true );
      editParameter ( paramOneShotRev, true );
      editParameter ( paramLoopFwd, true );
      editParameter ( paramLoopRev, true );

      setParameterValue ( paramOneShotFwd, 1.0f );
      setParameterValue ( paramOneShotRev, 0.0f );
      setParameterValue ( paramLoopFwd, 0.0f );
      setParameterValue ( paramLoopRev, 0.0f );

      fSwitchFwd->setDown ( true );
      fSwitchRev->setDown ( false );
      fSwitchLoopFwd->setDown ( false );
      fSwitchLoopRev->setDown ( false );

      editParameter ( paramOneShotFwd, false );
      editParameter ( paramOneShotRev, false );
      editParameter ( paramLoopFwd, false );
      editParameter ( paramLoopRev, false );
      break;
    }
    case paramOneShotRev:
    {
      p_OneShotFwd[currentSlice] = 0;
      p_OneShotRev[currentSlice] = 1;
      p_LoopFwd[currentSlice]    = 0;
      p_LoopRev[currentSlice]    = 0;

      editParameter ( paramOneShotFwd, true );
      editParameter ( paramOneShotRev, true );
      editParameter ( paramLoopFwd, true );
      editParameter ( paramLoopRev, true );

      setParameterValue ( paramOneShotFwd, 0.0f );
      setParameterValue ( paramOneShotRev, 1.0f );
      setParameterValue ( paramLoopFwd, 0.0f );
      setParameterValue ( paramLoopRev, 0.0f );

      fSwitchFwd->setDown ( false );
      fSwitchRev->setDown ( true );
      fSwitchLoopFwd->setDown ( false );
      fSwitchLoopRev->setDown ( false );

      editParameter ( paramOneShotFwd, false );
      editParameter ( paramOneShotRev, false );
      editParameter ( paramLoopFwd, false );
      editParameter ( paramLoopRev, false );
      break;
    }
    case paramLoopFwd:
    {
      p_OneShotFwd[currentSlice] = 0;
      p_OneShotRev[currentSlice] = 0;
      p_LoopFwd[currentSlice]    = 1;
      p_LoopRev[currentSlice]    = 0;

      editParameter ( paramOneShotFwd, true );
      editParameter ( paramOneShotRev, true );
      editParameter ( paramLoopFwd, true );
      editParameter ( paramLoopRev, true );

      setParameterValue ( paramOneShotFwd, 0.0f );
      setParameterValue ( paramOneShotRev, 0.0f );
      setParameterValue ( paramLoopFwd, 1.0f );
      setParameterValue ( paramLoopRev, 0.0f );

      fSwitchFwd->setDown ( false );
      fSwitchRev->setDown ( false );
      fSwitchLoopFwd->setDown ( true );
      fSwitchLoopRev->setDown ( false );

      editParameter ( paramOneShotFwd, false );
      editParameter ( paramOneShotRev, false );
      editParameter ( paramLoopFwd, false );
      editParameter ( paramLoopRev, false );
      break;
    }
    case paramLoopRev:
    {
      p_OneShotFwd[currentSlice] = 0;
      p_OneShotRev[currentSlice] = 0;
      p_LoopFwd[currentSlice]    = 0;
      p_LoopRev[currentSlice]    = 1;

      editParameter ( paramOneShotFwd, true );
      editParameter ( paramOneShotRev, true );
      editParameter ( paramLoopFwd, true );
      editParameter ( paramLoopRev, true );

      setParameterValue ( paramOneShotFwd, 0.0f );
      setParameterValue ( paramOneShotRev, 0.0f );
      setParameterValue ( paramLoopFwd, 0.0f );
      setParameterValue ( paramLoopRev, 1.0f );

      fSwitchFwd->setDown ( false );
      fSwitchRev->setDown ( false );
      fSwitchLoopFwd->setDown ( false );
      fSwitchLoopRev->setDown ( true );

      editParameter ( paramOneShotFwd, false );
      editParameter ( paramOneShotRev, false );
      editParameter ( paramLoopFwd, false );
      editParameter ( paramLoopRev, false );
      break;
    }
    case paramSliceMode:
    {
      fLabelsBoxSliceModeSlider->setSelectedIndex ( ( int ) value );
      setParameterValue ( paramSliceMode, value );
      slicemethod = value;
      if ( !slicemethod )
        {
          createSlicesRaw ();
        }
      else
        {
          createSlicesOnsets ();
        }

      break;
    }
    case paramLoadSample:
    {
      filebrowseropts.title = "Load audio file";
      filebrowseropts.startDir = directory.c_str();
      getParentWindow().openFileBrowser ( filebrowseropts );

      if ( sample_is_loaded )
        {
          fSwitchLoadSample->setDown ( true );
        }
      else
        {
          fSwitchLoadSample->setDown ( false );
        }
      break;
    }


    } // switch (buttonId)

  // process the grid

  if ( buttonId >= paramSwitch01 && buttonId <= paramSwitch16 )
    {
      for ( uint32_t i = paramSwitch01, j=0; i <= paramSwitch16; ++i,++j )
        {
          editParameter ( i, true );
          setParameterValue ( i, i == buttonId ? 1.0f : 0.0f );
          fGrid[j]->setDown ( i == buttonId );
          if ( i == buttonId )
            {
              currentSlice = j;
              recallSliceSettings ( j );
            }
          editParameter ( i, false );
        }
    }

//
  repaint();
}

void NinjasUI::onNanoDisplay()
{
  const float width = getWidth();
  const float height = getHeight();

  beginPath();

  fillColor ( Color ( 227,222,219, 255 ) );

  rect ( 0, 0, width, height );
  fill();

  closePath();

  // waveform display back

  beginPath();

  fillColor ( Color ( 179,179,179, 255 ) );

  rect ( 30, 50, 1140, 350 );
  fill();

  closePath();

  if ( sample_is_loaded )
    {
      drawSlices();
      drawWaveform();
      drawOnsets();
      drawRuler();
    }
}

void NinjasUI::drawSlices()
{
  double view = waveView.end - waveView.start; // set these when zooming in
  double foo =  display_length / view;

  int firstSlice = 0, lastSlice = 0;
  while ( a_slices[firstSlice].sliceEnd < waveView.start )
    {
      firstSlice++;
    }

  for ( int i = 0; i < slices ; i++ )
    {
      if ( a_slices[lastSlice].sliceStart < waveView.end )
        lastSlice++;
    }

  for ( uint left,right; firstSlice < lastSlice; firstSlice++ )
    {
      beginPath();

      if ( a_slices[firstSlice].sliceStart < waveView.start )
        left = 0;
      else
        left = ( a_slices[firstSlice].sliceStart - waveView.start ) * foo;
      
      if ( a_slices[firstSlice].sliceEnd > waveView.end )
        right = 1140;
      else
        right = ( a_slices[firstSlice].sliceEnd - waveView.start ) * foo;
      
      rect ( left+display_left,display_top,right - left,display_height*2 );
      if ( a_slices[firstSlice].color )
        fillColor ( 179,179,179,255 );
      else
        fillColor ( 150,150,150,255 );
      fill();
      closePath();
    }
}


void NinjasUI::drawOnsets()
{
  double view = waveView.end - waveView.start;
  double foo =  display_length / view;
  beginPath();
  strokeColor ( 255,127,0,255 );
  strokeWidth ( 0.8f );
  for ( std::vector<uint_t>::iterator it = onsets.begin() ; it != onsets.end(); ++it )
    {
      auto onset = *it;
      if ( onset >= waveView.start && onset <= waveView.end )
        {
          int display_onset_x = ( double ) ( onset - waveView.start ) * foo;
          std::cout << "display_onset_x =" << display_onset_x << std::endl;
          display_onset_x += display_left;
          for ( int i = display_top; i < display_bottom; i +=10 )
            {
              moveTo ( display_onset_x, i );
              lineTo ( display_onset_x, i+4 );
              moveTo ( display_onset_x, i+6 );
              lineTo ( display_onset_x, i+10 );
            }
        }
    }
  stroke();
  closePath();
}



void NinjasUI::loadSample ( String fp )
{
  std::cout << "loading sample " << fp << std::endl;
  //int  iIndex {0};
  //float fIndex {0};
  double samplerate = getSampleRate();

  SndfileHandle fileHandle ( fp , SFM_READ,  SF_FORMAT_WAV | SF_FORMAT_FLOAT , 2 , samplerate );
  sampleSize = fileHandle.frames();
  sampleChannels   = fileHandle.channels();
  if ( sampleSize == 0 )
    {
      sample_is_loaded = false;
      return;
    }
  sample_is_loaded =true;
  //fSwitchLoadSample->setDown ( true );
  //float samples_per_pixel = ( float ) ( sampleSize * sampleChannels ) / ( float ) lcd_length;

  sampleVector.resize ( sampleSize * sampleChannels );
  fileHandle.read ( &sampleVector.at ( 0 ) , sampleSize * sampleChannels );

  // display height = 350
  // store waveform as -175 to  175 integer

  waveform.resize ( 0 ); // clear waveform

  if ( sampleChannels == 2 ) // sum to mono
    {

      for ( uint i=0, j=0 ; i <= sampleSize; i++ )
        {
          float sum_mono = ( sampleVector[j] + sampleVector[j+1] ) * 0.5f;
          waveform.push_back ( sum_mono * 175.0f );
          j+=2;
        }
    }
  else
    {
      waveform.resize ( sampleSize );
      for ( uint i=0; i < sampleVector.size(); i++ )
        {
          waveform[i] =  sampleVector[i] * 175.0f;
        }
    }

  waveView.start = 0;
  waveView.end = sampleSize;
  waveView.zoom = 1.0f;
  waveView.max_zoom = float ( sampleSize ) / float ( display_width );

  //std::cout << sampleSize << " | " << waveform.size() << " | " << waveView.end <<  std::endl;

  getOnsets ( sampleSize ,sampleChannels, sampleVector, onsets );
  if ( !slicemethod )
    {
      createSlicesRaw ();
    }
  else
    {
      createSlicesOnsets ();
    }
  repaint();
  return;

}

void NinjasUI::createSlicesOnsets ()
{
  if ( sampleSize == 0 )
    {
//         std::cout << "no sample loaded" << std::endl;
      return;
    }
  long double sliceSize = ( long double ) sampleSize / ( long double ) slices;

  // raw slicing
  for ( int i = 0 ; i < slices; i++ )
    {
      int64_t start = ( ( int ) i * sliceSize );
      int64_t end = ( ( ( int ) ( i+1 ) * sliceSize ) - 1 );
      // map to nearest onset
      int64_t onset_start = find_nearest ( onsets,start );
      int64_t onset_end = find_nearest ( onsets,end )-1;

      a_slices[i].sliceStart = onset_start;
      a_slices[i].sliceEnd = onset_end;
      // set end of last slice to end of sample
      if ( i == slices -1 )
        {
          a_slices[i].sliceEnd = end;
        }

    }
}

int64_t NinjasUI::find_nearest ( std::vector<uint_t> & haystack, uint_t needle )
{
  auto distance_to_needle_comparator = [&] ( int64_t && a,  int64_t && b )
  {
    return abs ( a - needle ) < abs ( b - needle );
  };

  return *std::min_element ( std::begin ( haystack ), std::end ( haystack ), distance_to_needle_comparator );
}

std::string NinjasUI::dirnameOf ( const std::string& fname )
{
  size_t pos = fname.find_last_of ( "\\/" );
  return ( std::string::npos == pos )
         ? ""
         : fname.substr ( 0, pos );
}

void NinjasUI::recallSliceSettings ( int slice )
{
  setParameterValue ( paramAttack, p_Attack[slice] );
  fKnobAttack->setValue ( p_Attack[slice] );
  setParameterValue ( paramDecay,  p_Decay[slice] );
  fKnobDecay->setValue ( p_Decay[slice] );
  setParameterValue ( paramSustain, p_Sustain[slice] );
  fKnobSustain->setValue ( p_Sustain[slice] );
  setParameterValue ( paramRelease, p_Release[slice] );
  fKnobRelease->setValue ( p_Release[slice] );
  setParameterValue ( paramOneShotFwd, p_OneShotFwd[slice] );
  fSwitchFwd->setDown ( p_OneShotFwd[slice] == 1.0f );
  setParameterValue ( paramOneShotRev,  p_OneShotRev[slice] );
  fSwitchRev->setDown ( p_OneShotRev[slice] == 1.0f );
  setParameterValue ( paramLoopFwd, p_LoopFwd[slice] );
  fSwitchLoopFwd->setDown ( p_LoopFwd[slice] == 1.0f );
  setParameterValue ( paramLoopRev, p_LoopRev[slice] );
  fSwitchLoopRev->setDown ( p_LoopRev[slice] == 1.0f );

  repaint();
}

void NinjasUI::getOnsets ( int64_t size, int channels, std::vector<float> & sampleVector, std::vector<uint_t> & onsets )
{
  // temp sample vector
  std::vector<float> tmp_sample_vector;
  onsets.resize ( 0 ); // wipe onsets
  uint_t samplerate = getSampleRate();
  int hop_size = 256;
  int win_s = 512;

  fvec_t ftable;               // 1. create fvec without allocating it
  intptr_t readptr = 0;
  ftable.length = hop_size;    // 2. set ftable length
  fvec_t * out = new_fvec ( 2 ); // output position

  if ( channels == 2 ) // create mono sample
    {
      for ( int i=0, j=0 ; i <= size; i++ )
        {
          // sum to mono
          float sum_mono = ( sampleVector[j] + sampleVector[j+1] ) * 0.5f;
          tmp_sample_vector.push_back ( sum_mono );
          j+=2;
        }
    }
  else
    {
      tmp_sample_vector = sampleVector;
    }

  // create onset object/
  aubio_onset_t  * onset = new_aubio_onset ( "complex", win_s, hop_size, samplerate );
  while ( readptr < tmp_sample_vector.size() )
    {
      ftable.data = &tmp_sample_vector[readptr];
      aubio_onset_do ( onset , &ftable, out );
      if ( out->data[0] != 0 )
        {
          onsets.push_back ( aubio_onset_get_last ( onset ) );
        }
      readptr += hop_size;
    }
  del_aubio_onset ( onset );
  // del_fvec ( &ftable );
  // del_fvec ( out );
  aubio_cleanup();
}

void NinjasUI::createSlicesRaw ()
{
  long double sliceSize = ( long double ) ( sampleSize ) / ( long double ) slices;
  bool color {true};

  for ( int i = 0 ; i < slices; i++ )
    {
      a_slices[i].sliceStart = i * sliceSize;
      a_slices[i].sliceEnd = ( i+1 ) * sliceSize  - 1 ;
      std::cout << "Slice # "<< i << " : Start " << a_slices[i].sliceStart / samplerate << std::endl;
      a_slices[i].color = color;
      color = !color;
    }
}

void NinjasUI::drawWaveform()
{
//  waveView.end = 1140;
  double view = waveView.end - waveView.start; // set these when zooming in
  double samples_per_pixel =  view / display_length;
  float fIndex;
  uint iIndex;

  // draw center line
  beginPath();
  strokeColor ( 0,0,0,255 );
  moveTo ( display_left,display_center );
  lineTo ( display_right,display_center );
  stroke();
  closePath();

  beginPath();
  strokeColor ( 80,45,22,255 );
  strokeWidth ( 1.0f );

  moveTo ( display_left,display_center );
  for ( uint16_t i = 0 ; i < display_length ; i++ )
    {
      fIndex = float ( waveView.start ) + ( float ( i ) * samples_per_pixel );
      iIndex = fIndex;
      auto minmax = std::minmax_element ( waveform.begin() + iIndex, waveform.begin() + iIndex + int ( samples_per_pixel ) );
      uint16_t min = *minmax.first + display_center;
      uint16_t max = *minmax.second + display_center;

      lineTo ( i+display_left,min );
      lineTo ( i+display_left,max );
    }
  stroke();
  closePath();


}

void NinjasUI::drawRuler()
{
  uint view = waveView.end - waveView.start; // set these when zooming in
  double samples_per_pixel = double ( view ) / double ( display_length );
  double time_per_pixel = samples_per_pixel / samplerate;
  double round_up = 0.1; // do something clever here
  double wave_start_time = double ( waveView.start ) / samplerate;
  double wave_end_time = double ( waveView.end ) / samplerate;
  double wave_length_time = wave_end_time - wave_start_time;
  int gap = wave_length_time / 10;
  int incms;
  if ( gap > 0 )
    {
      incms = 1000;
      if ( gap > 0 )
        {
          incms *= 5;
          gap /= 5;
        }
      if ( gap > 0 )
        {
          incms *= 2;
          gap /= 2;
        }
      if ( gap > 0 )
        {
          incms *= 6;
          gap /= 6;
        }
      if ( gap > 0 )
        {
          incms *= 5;
          gap /= 5;
        }
      if ( gap > 0 )
        {
          incms *= 2;
          gap /= 2;
        }
      if ( gap > 0 )
        {
          incms *= 6;
          gap /= 6;
        }
      while ( gap > 0 )
        {
          incms *= 10;
          gap /= 10;
        }
      round_up = double ( incms/1000 );
    }
  else
    {
      incms = 1;
      int ms = ( wave_length_time/10 ) * 1000;
      if ( ms > 0 )
        {
          incms *= 10;
          ms /= 10;
        }
      if ( ms > 0 )
        {
          incms *= 10;
          ms /=10;
        }
      if ( ms > 0 )
        {
          incms *= 5;
          ms /= 5;
        }
      if ( ms > 0 )
        {
          incms *= 2;
          ms /= 2;
        }
      round_up = double ( incms/1000.0 );
    }

  double time = ceil ( ( 1.0 / round_up ) * wave_start_time );
  time= time / ( 1.0 / round_up );
  double timeX = display_left;
  std::string sTime;
  fontFaceId ( fNanoFont );
  textAlign ( ALIGN_CENTER|ALIGN_TOP );
  fillColor ( Color ( 0.0f, 0.0f, 0.0f ) );
  fontSize ( 9 );
  beginPath();
  strokeColor ( 0,0,0,255 );
  strokeWidth ( 1.0f );
  while ( time < wave_end_time )
    {
      timeX = ( time-wave_start_time ) / time_per_pixel + display_left;
      sTime = toTime ( time, round_up );
      if ( ( timeX - 15 ) >= display_left && ( timeX+15 ) <= display_right )
        textBox ( timeX - 15 , display_top + 10 , 30.0f, sTime.c_str(), nullptr );

      moveTo ( timeX, display_top );
      lineTo ( timeX, display_top + 10 );
      time = time + round_up;
    }
  stroke();
  closePath();
}

std::string NinjasUI::toTime ( double time, double round_up )
{
  int hour,min,sec,ms,iTime;
  std::string sHour,sMin,sSec,sMs;
  iTime = time;
  hour = iTime / 3600;
  min = iTime / 60 - hour * 60;
  sec = iTime - hour * 3600 - min * 60 ;
  ms = ( time-iTime ) * 100;
  int iRound = round_up * 100;
  switch ( iRound )
    {
    case 100:
    {
      sMs = ".0";
      break;
    }
    case 50:
    {
      sMs = "."+ std::to_string ( ms );
      sMs = sMs.substr ( 0,2 );
      break;
    }

    case 10:
    {
      sMs = std::to_string ( ms );
      sMs = "." + sMs.substr ( 0,2 );
      break;
    }

    case 1:
    {
      sMs= "00" + std::to_string ( ms );
      sMs = "." + sMs.substr ( sMs.size()-2,3 );
      break;
    }

    }

  if ( hour )
    {
      sHour = std::to_string ( hour ) +":";

      sMin = "0" + std::to_string ( min );
      sMin = sMin.substr ( sMin.size()-2,2 ) + ":";

      sSec = "0" + std::to_string ( sec );
      sSec = sSec.substr ( sSec.size()-2,2 ) + ":";
      return sHour+sMin+sSec+sMs;
    }

  if ( min )
    {
      sMin = std::to_string ( min ) + ":";

      sSec = "0" + std::to_string ( sec );
      sSec = sSec.substr ( sSec.size()-2,2 );
      return sMin+sSec+sMs;
    }

  if ( sec )
    {
      sSec = std::to_string ( sec );
      return sSec+sMs;
    }

  if ( ms )
    {
      return "0" + sMs;
    }

  return "0.000";
}


bool NinjasUI::onMouse ( const MouseEvent& ev )
{
  if ( !mouseDragging )

    if ( ev.press && ev.button == 2 )
      {
        mouseDragging = true;
        mouseMoveWaveform = true;
        mouseX = ev.pos.getX()-display_left;
      }
  if ( ev.press && ev.button == 1 )
    {
      mouseDragging = true;
      mouseMoveWaveform = false;
      mouseX = ev.pos.getX()-display_left;
    }


  if ( !ev.press )
    {
      mouseDragging = false;
      mouseMoveWaveform = false;
    }


  return false;
}

bool NinjasUI::onScroll ( const ScrollEvent& ev )
{
  // is the pointer in the display
  int x = ev.pos.getX();
  int y = ev.pos.getY();
  if ( !display.contains ( x,y ) )
    return false; // get outta here

  if ( waveform.size() <= display_length )
    return false; // can't zoom anyway

  x -= display_left; // off set in pixels
  // find sample index mouse is hovering at
  // old zoom factor
  uint center = int ( pow ( waveView.max_zoom,waveView.zoom ) * ( float ( x ) ) + float ( waveView.start ) );
  // new zoom factor
  float delta = -ev.delta.getY() *0.05f;

  waveView.zoom += delta;
  if ( waveView.zoom < 0.0f )
    waveView.zoom = 0.0f;
  if ( waveView.zoom > 1.0f )
    waveView.zoom = 1.0f;
  float samples_per_pixel =  pow ( waveView.max_zoom,waveView.zoom );
  uint length = int ( samples_per_pixel * float ( display_width ) );
  waveView.start = int ( float ( center )  - ( float ( x )  *  samples_per_pixel ) );
  if ( waveView.start < 0 )
    waveView.start = 0;
  waveView.end = waveView.start+length;
  if ( waveView.end > waveform.size() )
    {
      waveView.end = waveform.size();
      waveView.start = waveView.end-length;
    }
  repaint();
  return true;
}

bool NinjasUI::onMotion ( const MotionEvent& ev )
{
  if ( !mouseDragging )
    {
      //std::cout << "not dragging" << std::endl;
      return false;
    }
  if ( mouseMoveWaveform )
    {
      if ( waveform.size() <= display_length )
        return false; // can't move anyway

      if ( waveView.zoom == 1.0f )
        return false;

      int x = ev.pos.getX();
      int y = ev.pos.getY();
      if ( !display.contains ( x,y ) )
        return false; // get outta here

      x -= display_left; // off set in pixels
      mouseDistance = x - mouseX;
      mouseX = x;
      if ( ( mouseDistance < 0 ) & ( waveView.end == waveform.size() ) )
        return false;

      float samples_per_pixel =  pow ( waveView.max_zoom,waveView.zoom );
      uint length = int ( samples_per_pixel * float ( display_width ) );
      waveView.start = waveView.start - int ( float ( mouseDistance )  *  samples_per_pixel );
      if ( waveView.start < 0 )
        waveView.start = 0;
      waveView.end = waveView.start+length;
      if ( waveView.end > waveform.size() )
        waveView.end = waveform.size();
      repaint();
    }


  return false;
}


/* ------------------------------------------------------------------------------------------------------------
 * UI entry point, called by DPF to create a new UI instance. */

UI* createUI()
{
  return new NinjasUI();
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO



