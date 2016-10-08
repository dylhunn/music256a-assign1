// Music 256a / CS 476a | fall 2016
// CCRMA, Stanford University
//
// Author: Dylan Hunn
// Assignment Author: Romain Michon
// Description: Additive JUCE sine wave synthesizer with harmonic series and detuning!

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Sine.h"

class MainContentComponent :
    public AudioAppComponent,
    private Slider::Listener,
    private ToggleButton::Listener
{
public:
    MainContentComponent() : gain (0.0), samplingRate(0.0)
    {
        // configuring frequency slider and adding it to the main window
        addAndMakeVisible (frequencySlider);
        frequencySlider.setRange (50.0, 5000.0);
        frequencySlider.setSkewFactorFromMidPoint (500.0);
        frequencySlider.setValue(1000); // will also set the default frequency of the sine osc
        frequencySlider.addListener (this);
        
        // configuring frequency label box and adding it to the main window
        addAndMakeVisible(frequencyLabel);
        frequencyLabel.setText ("Frequency", dontSendNotification);
        frequencyLabel.attachToComponent (&frequencySlider, true);
        
        // configuring gain slider and adding it to the main window
        addAndMakeVisible (gainSlider);
        gainSlider.setRange (0.0, 1.0);
        gainSlider.setValue(0.5); // will alsi set the default gain of the sine osc
        gainSlider.addListener (this);
        
        // configuring gain label and adding it to the main window
        addAndMakeVisible(gainLabel);
        gainLabel.setText ("Gain", dontSendNotification);
        gainLabel.attachToComponent (&gainSlider, true);
        
        // make visible on/off buttons for each tone in the harmonic series
        for (int i = 0; i < kHarmonicSeriesTones; i++) {
            addAndMakeVisible(harmonicSeriesOnOffButtons[i]);
            harmonicSeriesOnOffButtons[i].addListener(this);
            addAndMakeVisible(harmonicSeriesOnOffLabels[i]);
            String buttonLabel = "Harmonic series tone " + std::to_string(i + 1);
            harmonicSeriesOnOffLabels[i].setText(buttonLabel, dontSendNotification);
            harmonicSeriesOnOffLabels[i].attachToComponent(
                &(harmonicSeriesOnOffButtons[i]), true);
            
        }
        
        // configuring detuning label/slider and adding it to the main window
        addAndMakeVisible(detuneLabel);
        detuneLabel.setText("Detune ratio", dontSendNotification);
        detuneLabel.attachToComponent(&detuneSlider, true);
        addAndMakeVisible(detuneSlider);
        detuneSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        detuneSlider.setRange (.95, 1.05);
        detuneSlider.setValue(1);
        detuneSlider.addListener(this);
        
        setSize (600, 400);
        nChans = 1;
        setAudioChannels (0, nChans); // no inputs, one output
    }
    
    ~MainContentComponent()
    {
        shutdownAudio();
    }
    
    void resized() override
    {
        // placing the UI elements in the main window
        // getWidth has to be used in case the window is resized by the user
        const int sliderLeft = 80;
        frequencySlider.setBounds (sliderLeft, 10, getWidth() - sliderLeft - 20, 20);
        gainSlider.setBounds(sliderLeft, 40, getWidth() - sliderLeft - 20, 20);
        int horizontalButtonPosition = 200;
        // place the custom harmonic frequency buttons below the on/off button
        int nextVerticalPosition = 70; // Start below the first controls
        for (int i = 0; i < kHarmonicSeriesTones; i++) {
            harmonicSeriesOnOffButtons[i].setBounds(horizontalButtonPosition,
                                                    nextVerticalPosition, getWidth() - sliderLeft - 20, 20);
            nextVerticalPosition += 30;
        }
        detuneSlider.setBounds(sliderLeft, nextVerticalPosition, sliderLeft + 80, 60);
        
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        if (samplingRate > 0.0){
            if (slider == &frequencySlider){
                for (int i = 0; i < kHarmonicSeriesTones; i++) {
                    // multiply to find the appropriate otne in the harmonic series
                    float newFrequency = frequencySlider.getValue() * (i + 1);
                    sine[i].setFrequency(newFrequency);
                    detuneWaves[i].setFrequency(newFrequency * detuneSetting);
                }
            } else if (slider == &gainSlider){
                gain = gainSlider.getValue();
            } else if (slider == &detuneSlider) {
                detuneSetting = detuneSlider.getValue();
                for (int i = 0; i < kHarmonicSeriesTones; i++) {
                    float newFrequency = (frequencySlider.getValue() * (i + 1));
                    detuneWaves[i].setFrequency(newFrequency * detuneSetting);
                }
            }
        }
    }
    
    void buttonClicked (Button* button) override
    {
        // determine which tone  button was clicked
        for (int i = 0; i < kHarmonicSeriesTones; i++) {
            if (button != harmonicSeriesOnOffButtons + i) continue;
            if (harmonicSeriesOnOffButtons[i].getToggleState()) onOff[i] = 1; // toggle the audio on
            else onOff[i] = 0; // or off
            break; // we are finished searching for the button
        }
    }
    
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        samplingRate = sampleRate;
        for (int i = 0; i < kHarmonicSeriesTones; i++) {
            sine[i].setSamplingRate(sampleRate);
            detuneWaves[i].setSamplingRate(sampleRate);
        }
    }
    
    void releaseResources() override
    {
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // getting the audio output buffer to be filled
        float* const buffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        
        // computing one block
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            buffer[sample] = 0;
            for (int i = 0; i < kHarmonicSeriesTones; i++) {
                if (onOff[i] == 1) {
                    buffer[sample] += sine[i].tick() * gain;
                    buffer[sample] += detuneWaves[i].tick() * gain;
                }
            }
        }
    }
    
    
private:
    // Data constants
    static const int kHarmonicSeriesTones = 7; // the number of tones in the series to support
    
    // UI Elements
    Slider frequencySlider;
    Slider gainSlider;
    Slider detuneSlider;
    ToggleButton harmonicSeriesOnOffButtons[kHarmonicSeriesTones];
    Label harmonicSeriesOnOffLabels[kHarmonicSeriesTones];
    
    Label frequencyLabel, gainLabel, onOffLabel, detuneLabel;
    
    Sine sine[kHarmonicSeriesTones]; // the sine wave oscillators
    Sine detuneWaves[kHarmonicSeriesTones]; // the sine wave oscillators
    int onOff[kHarmonicSeriesTones]; // turn the various tones on or off
    
    // Global Variables
    float gain;
    float detuneSetting;
    int samplingRate, nChans;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
