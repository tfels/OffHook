# OffHook

This is a small programm that uses the Jabra SDK for emulating a softphone.

It is useful if you have a non-Jabra aware communication client (e.g. **GoToMeeting**) and you want to control the off-hook functionality of your headset.
If you do not set the headset to off-hook state, an incomming call will capture the headset and your computer audio communication is interrupted.

## Prerequisites

You'll need the two libraies `JabraSDK.dll` and `libjabra.dll` from the [Jabra SDK](https://developer.jabra.com/site/global/sdks/windows/index.gsp) for communicating with a Jabra headset.

## Command line options

* `-minimized` to start minimized to tray icon (if configured).

## Dependencies

This project uses an external component for JSON parsing. The source code is included as a git submodule directly from its original git repository at https://github.com/udp/json-parser.

The JSON parser is `Copyright (C) 2012, 2013 James McLaughlin et al.  All rights reserved.`
 
## Authors

* Tim Felser