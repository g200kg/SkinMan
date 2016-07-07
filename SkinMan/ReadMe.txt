0.999l 20120728
* Improve the screen quality when zoomout

0.999k 20110920
* BugFix: Cannot cancel opening another file by 'Are you sure?' dialog.
* BugFix: 'Color Window' position is not saved in some cases.
* BugFix: Drives are not displayed in FileOpen-MyComputer dialogs (WinXP).

0.999j
* BugFix: Not reflect to render when the primitive Size is changed by PropWindow value.
* BugFix: Primitive position may move unintendedly by Group/Ungroup
* Added 'Copy Primitive coodinate to clipboard' button

0.999i
* BugFix: Pixel color accuracy improved especially using Gradation
* BugFix: UseTextureAlpha is not effective when Diffuse is not zero
* Added 'Smoother' parameter that make Gradation more smooth, applying splined curve

0.999h 20110616
* BugFix: Primitive cannot move to parent (by dragging or menu-command) 

0.999g 20110618
* BugFix: Primitives are not focused after paste (CTRL-C/CTRL-V)
* BugFix: Color value of the boundary pixel between PrimitiveBody&DropShadow is not correct
* BugFix: In some case, the position is not accurate (+-0.5pix) when primitive move by dragging
* BugFix: Menu item enable/disable control is wrong about 'Edit->Primitive->Extract Image'
* Support Texture rotation / scaleX,Y / OffsetX,Y
* Support Texture alpha to cut-out
* Add Textures 'Punching Metal' and 'Punching Slits'
* NOTE: from this version, SSE supported CPU is needed

0.999f 20110616
* BugFix: Window Z order control is not appropriate when 'ToolsAlwaysOnFront' is on
* Default .bmp file format is changed to 24bpp, 32bpp w/alpha bmp is also supported
* Export file format is now selected by the 'File Type' of the file export dialog
  (in default, bmp=24bpp, png=w/alpha)
* Texture file's alpha is effective
* Texture name sort

0.999e 20110612
* BugFix: crash when some case of Undo/Redo
* BugFix: many bugs about TreeView manipulation (including crash when multi-select&drag)
* BugFix: Resize Keeping aspect (pressing Shift) is now work normal
* BugFix: in some case, FileOpenDialog may not come front when ToolsAlwaysOnFront is on.
* BugFix: Tab key enabled in Color window.
* TreeView dragging operation performance is improved (only for single item dragging)
* Texture file format is now support not only .bmp but .png .jpg .gif
* The texture is now placed adjusting the center of the object instead of TopLeft corner
* Added a texture file 'Circle.jpg'
* Added 'Extract Images/Knobs to File' function to Right Click menu.
* Added a sample file 'VeeMax.skin'

0.999d 20110524
* BugFix: Changing Font/Bold/Italic/SmallFontOpt cause unexpected text size/aspect modification

0.999c 20110522
* BugFix: Texture selection combobox is misaligned
* BugFix: TreeView item click with Shift/Ctrl behavior is wrong in some case
                (now Shift=AddRange / Ctrl=Toggle)
* BugFix: 'Move primitives by cursor-key when the focus on Tree-View' with ShiftKey behavior

0.999b 20110521
* BugFix: floating value handling by cursor up/down
* BugFix: cyclic focus change when primitives are overlayed on same position
* Add setup option of 'Move primitives by cursor-key when the focus on Tree-View'.
* Add default setting of Grid enable/display to setup dialog
* Allow bulk-edit of multiple selected primitives

Ver 0.999a 20110313
* BugFix: The pure color (ex. 255,0,255) is now exported exactly.
* OpenType font - PostScript type (.otf) font is now supported.

Ver 0.999 20110225
* Slant Parameter supported.
* Border option 'External/Internal/Both' is added.
* BugFix: Layout breaking by resizing if a group include angled primitives.

Ver 0.998 20101122
* Guide line display when select a primitive
* BugFix&Improved Group/UnGroup behavior (will not affect to lock status)
* Improved threading (at least marker&guidelines are displayed quickly)
* Some help text display on the status bar

Ver 0.997 20101030
* BugFix: Crush in some case, especially move a primitive by wheel after Undo.
* Improved hehavior of multiple selection on the Tree window.

Ver 0.996a 20100614
* BugFix: Properties window cannot open on Win2000

Ver 0.996 20100606
* BugFix: crush in some cases, especially with the texture related operation.

Ver 0.995 20100412
* Fix crush when move primitives in some cases
* Support decimal places for 'Emboss depth' parameter
* Text-path function (imcompleted).

Ver 0.99d 20091209
* startup time improvement (with splash screen) when many textures installed
* FontSize 'Px' / 'Pt' switch is added
* 'SmallFontOpt' option for small font quality improvement (work only when aspect=100% & no-mirrored & no-rotation)

Ver 0.99c 20091207
* Support negative values for 'Size' that will mirror any primitives
* 'Font spacing' parameter is added
* Multi-language support for any language if you prepare the language-file

Ver 0.99b 20091130
* BugFix: Cant edit values in some editboxes
* BugFix: many minor bugfixes
* Alpha gradation support for Image primitives
* Performance optimization
* multilanguage support for Japanese

Ver 0.99a 20091124
* BugFix: another memory leaks.
* Project load time reduced.
* Undo level setting added.

Ver 0.99 20091121
* BugFix: fix the memory leaks. that may cause crash when edit huge .skin file.
* support drag&drop the image-file (.bmp / .png / .jpg / .gif / .knob / .skin) from Explorer to SkinMan canvas.
* Library file ".sklib" also be supported drag&drop operation. 

Ver 0.98d 20091117
* The canvas can be divided as frames. It is useful for making some type of switches by SkinMan
* Now .skin file is importable as a 'Image' object (like a .knob or other image-file)
* BugFix: Pixel value is not accurate for pure colors (e.g. pure magenta 255,1,255)
* Change to use 24bpp instread of 32bpp when Export PNG without transparent

Ver 0.98c 20090724
* BugFix: run-time error caused by memory leak 

Ver 0.98b 20090703
* text autosize on/off option added 

Ver 0.98 20090703
* many bug fixes
* Align/Distribution
* Changable primitive type (except for shape/image)
* individual Emboss depth for border/fill

0.97g
* gif export improved if used color <=256

Ver 0.97
* Lines (radiate/horizontal/vertical) primitive suported.
* LineCap parameter for Shape.
* Preview added for Fonts / Textures.
* Export/Import libs (*.sklib) support.
  (to help reuse partial objects)
* Borders with Shape- operation supported.
* BugFix: Twist for Polygon is not properly processed.
* Multiple selection on TreeView (using Shift/Control+Click).
* AroowKeys and Shift+ArrowKeys for move selected Primitives on canvas.
* Color palette embedded to .skin and Save/Load support
* BugFix: pipette tool hungs in some cases.

Ver 0.96
* BugFix: losing texture/image in some cases.
* Polygon curve/twist parameters are added
* ellipse start/stop parameters are added
* 'Tools always in front' option for setup menu is added
* combobox space is extended for Text/Font/TextureType
* improved group resizing accuracy
* Font aspect parameter is added. Accurate font resizing is supported.
* "Edit Image/Knob" , "Insert new Knob" menu is added (cooperating with KnobMan)

Ver 0.95 20090608
* Tree / Properties window is separated from main window. (USE F5 / F6 / F7 for display control)
  (long TextureType/Text/FontName can be displayed if you widen the window)
* Group/Ungroup supported (Use right click menu)
* Dual specular parameters are added
* Trimming tool is added
* FileOpen by Drag&Drop / double clicking
* Fractional EmbossWidth support
* Pipette tool, (can pick from any screen)
* focus changing by TAB key on properties window
* BugFix: position cannot be reloaded if you place object to negative X position.
          and many minor bugfixes.

Ver 0.94 20090119
* BugFix: Undo/Redo cant restore the background/workspace colors
* BugFix: crash in some cases (undo/redo+object-selecting, properties change for deleted obj)
* Improved ColorPicker. Color selection is realtimely reflected to the screen.

Ver 0.93 20090112
* BugFix: cannot load .skin files on WindowXP
* Restore the fix on 0.92 about texts because of a side-effect that makes extra squares.

2009/01/02 Ver 0.92
* BugFix: text following after space cannot displayed in some fonts
* BugFix: ShadowDiffuse param are set to abnormal value after the primitive focused

2008/12/28 Ver 0.91
* BugFix: Tree infomation lost on load .skin
* BugFix: Textures lost on old version .skin load
* BugFix: DropShadow/InsideShadow parameters can be set independently.

2008/12/28 Ver 0.90
* Image files and Texture files are embedded to .skin file
        (need only .skin for data sharing)
* .knob file saved by KnobMan1.30 or later is supported as a image
* Number of the Texture bitmap limitaion (upto64) is removed
* 'AntiAlias' on/off option is added

Ver 0.89
* SelectAll / SelectVisible commands added
* Undo point fix for Paste/Import/Trimming/TrimmingVisible
* Visible Control for child objects fixed

Ver 0.88
* newly created/pasted primitives are placed next to the current focus
* mouse cursor image revised
* primitive 'Solo' visible control supported
* mouse pointer coordinates display added
* 'Canvas'-'Triming' / 'Canvas'-'TrimingVisible' support
* 'File'-'Export' / 'File'-'Import' support
* copy/paste between multiple instances