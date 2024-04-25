# Eject IDB Plugin for IDA

The `eject_idb` plugin was conceived after discussions with friends about the frustrations of IDA's UI hanging without warning or due to programmer errors (like an infinite-loop in their plugin or scripts), leaving no opportunity to save the ongoing work. Worry no more! `eject_idb` might *potentially* save the day.

It operates by spawning a separate thread that waits for a per database semaphore name to be signaled. When signaled, the plugin will save your current IDB as `original-name-ejected.ext`, then offers to terminate IDA on your behalf.

Long story short, `eject_idb` can be used as the last ditch effort before to save your work before IDA crashes.

Build this plugin using [ida-cmake](https://github.com/0xeb/ida-cmake) or get binaries from [here](https://github.com/0xeb/allthingsida/releases/tag/04232024).

**Note: this 'ejection' mechanism is not foolproof and might not work in all scenarios**

## Usage

1. Install the plugin by copying the `eject_idb` directory to your IDA's `plugins` directory.			
1. The plugin will be active automatically when you open an IDB.						
1. At any time you want to 'eject' the IDB, just run the accompanying `eject_idb` executable and pass the full IDB path as an argument (case sensitive).
