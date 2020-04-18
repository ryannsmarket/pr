

set (INSPECTOR_SRC
    ${CMAKE_CURRENT_LIST_DIR}/alignSelect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/alignSelect.h
    ${CMAKE_CURRENT_LIST_DIR}/fontStyleSelect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fontStyleSelect.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorAmbitus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorAmbitus.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorArpeggio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorArpeggio.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorBarline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorBarline.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorBase.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorBeam.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorBeam.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorBend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorBend.h
    ${CMAKE_CURRENT_LIST_DIR}/inspector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorDynamic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorDynamic.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorElementBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorElementBase.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorFingering.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorFingering.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorFret.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorFret.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorGlissando.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorGlissando.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorGroupElement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorGroupElement.h
    ${CMAKE_CURRENT_LIST_DIR}/inspector.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorHairpin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorHairpin.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorHarmony.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorHarmony.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorImage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorImage.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorInstrchange.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorInstrchange.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorJump.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorJump.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorLasso.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorLasso.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorLetRing.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorLetRing.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorMarker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorMarker.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorMeasureNumber.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorMeasureNumber.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorNote.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorNote.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorNoteDot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorNoteDot.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorOttava.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorOttava.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorPalmMute.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorPalmMute.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorPedal.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorPedal.h
#    ${CMAKE_CURRENT_LIST_DIR}/inspectorplugin.cpp
#    ${CMAKE_CURRENT_LIST_DIR}/inspectorplugin.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorTextBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorTextBase.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorText.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorText.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorTextLineBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorTextLineBase.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorTextLine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorTextLine.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorTremoloBar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorTremoloBar.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorTrill.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorTrill.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorVibrato.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorVibrato.h
    ${CMAKE_CURRENT_LIST_DIR}/inspectorVolta.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inspectorVolta.h
    ${CMAKE_CURRENT_LIST_DIR}/offsetSelect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/offsetSelect.h
    ${CMAKE_CURRENT_LIST_DIR}/resetButton.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resetButton.h
    ${CMAKE_CURRENT_LIST_DIR}/scaleSelect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/scaleSelect.h
    ${CMAKE_CURRENT_LIST_DIR}/sizeSelect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sizeSelect.h
    ${CMAKE_CURRENT_LIST_DIR}/voicingSelect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/voicingSelect.h
    )

set (INSPECTOR_UI
    ${CMAKE_CURRENT_LIST_DIR}/align_select.ui
    ${CMAKE_CURRENT_LIST_DIR}/font_style_select.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_accidental.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_ambitus.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_arpeggio.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_articulation.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_barline.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_beam.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_bend.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_bracket.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_break.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_caesura.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_chord.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_clef.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_dynamic.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_element.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_empty.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_fermata.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_fingering.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_frametext.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_fret.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_glissando.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_group_element.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_hairpin.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_harmony.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_hbox.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_image.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_iname.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_instrchange.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_jump.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_keysig.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_lasso.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_letring.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_line.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_lyric.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_marker.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_notedot.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_note.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_ottava.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_palmmute.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_pedal.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_rest.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_sectionbreak.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_segment.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_slur.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_spacer.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_stafftext.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_stafftypechange.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_stem.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_tbox.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_tempotext.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_textlinebase.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_textline.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_text.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_timesig.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_tremolobar.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_tremolo.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_trill.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_tuplet.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_vbox.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_vibrato.ui
    ${CMAKE_CURRENT_LIST_DIR}/inspector_volta.ui
    ${CMAKE_CURRENT_LIST_DIR}/offset_select.ui
    ${CMAKE_CURRENT_LIST_DIR}/scale_select.ui
    ${CMAKE_CURRENT_LIST_DIR}/size_select.ui
    ${CMAKE_CURRENT_LIST_DIR}/voicing_select.ui
    )









