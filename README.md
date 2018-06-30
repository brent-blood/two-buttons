# rowing-machine-reader
Push buttons to wirelessly change pages on an ebook reader!

I have a rowing machine. I should use it more than I do. I have a television mounted where I can see it,
and sometimes I watch television while I exercise. Sometimes I would rather read a book while I row, but
that's difficult. So I got to thinking... why not cobble together something that can simplify it down to
two buttons (_prev_ and _next_) attached to inputs on a microcontroller with wifi - and send a signal to
a computer that displays books on my television and responds to those commands by turning the pages.

So I made that. It actually does make the time go faster.

## Components

### ESP8266
I used an [Adafruit Feather Huzzah](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266) for the
final solution, but I prototyped with a cheap NodeMCU board. I made it battery powered, which would have
worked with the NodeMCU and a 9V battery, but I chose to use a lipo battery and the adafruit board has a
builtin lipo charger (which I didn't end up using). Use whatever.

### Push Buttons
I don't even know where I got these little buttons, but if you're reading this and want to do this, you
probably know where to get them already. Look at the pictures if you want to see them.

### Proto Board
I think I have [these](https://www.amazon.com/Double-Sided-Board-Prototype-Paxcoo/dp/B01N3161JP/ref=sr_1_3?ie=UTF8&qid=1530323740&sr=8-3&keywords=circuit+proto+board).
I use one for the main box (whatever size is big enough for all the components and fits in your enclosure)
that I have lashed to the center of the rowing machine handle and a smaller thin one for the button board
that I actually interact with.

### Screw Terminals
I didn't want to solder some of the components directly to each other, so this makes it easy to remove
the battery and plug in the wires that run to the button board. Don't use them if you don't want to.

### Wire
Make sure you have some wire.

### Battery
You won't want any power cables running from your rowing machine handle to the wall. Buy a battery. Depending
upon how long you want it to last, 9V batteries can run for many hours, but I got a lipo battery that
is rechargable. I got one of [these](https://www.adafruit.com/product/1578) along with a charger.

### Case
You need something to put the guts in that you can then lash down with zip ties or something. You'll need
to drill some holes in it for the wires to get out to the button board, as well as for the power
switch. If you wanted to be fancy, attach an LED to it to let you know when it's on or something. I
bought a bunch on Amazon for $2/each that shipped from China and it took like a month to get it. Whatever.

### Power Switch
Buy a 2 pin on/off switch that can be mounted to the case. Again, no idea where I got mine at (I raided
a bunch of Radioshacks when they were going out of business so probably one of them) but this should cost
like a quarter.

### A Mac with Kindle App and Content to Read
I really should spend some more time with this to get it working with something other than my mac laptop
but I'm lazy (hence needing something to read while exercising) and this was easy due to Applescript
being so simple to use and the presence of a well-written Amazon Kindle app with lots of content. I
could probably get this working with a Linux-powered arm SoC too via something like [xdotool](https://github.com/jordansissel/xdotool)
but for now this is what I got.

## Code

### rowing-machine-reader.cpp
This is my arduino sketch that polls for button pushes and sends UDP datagrams to the computer system
that is showing my book. Change the ESSID, wifi password, and host to suit your needs.

### rowing-machine-reader.py
This is the client that runs on my mac. It listens for UDP datagrams that contain "prev" and "next"
and then runs a short little AppleScript to send the keyboard events for left and right arrow to the
Kindle app. It's absurd how Apple made this sort of thing!

## Assembly
Check out the fritzing diagram. Assembly is pretty easy to get it functional on the bench... though
making it pretty is always a challenge.

## Questions You May Have (comments I don't care to explain again)
What about security? The network connection is not authenticated, and it's not encrypted. I'm using
it in my friggin basement on my own wifi and that's fine. If you use this in public, it's still
probably fine to not have it authenticated or encrypted because seriously nobody is going to mess
with it - but in the spirit of hacking, feel free to extend it.

Why didn't I use bluetooth or xbee or smoke signals or something else? I dunno. This is what I had
lying around.

Why is the arduino sketch file named rowing-machine-reader.cpp and not something that ends in
.ino? Why does it include a header file for Arduino.h? Because the arduino IDE is an aweful piece
of software and I made this using [PlatformIO](https://platformio.org/). The file was actually
called main.cpp when I copied it off so consider this an improvement. You're welcome.

How long did this take to make? I prototyped it in about half an hour on a breadboard. Then it sat
on my bench for about 3 weeks before I busted out the soldering iron and drill and made it work
for real.

How do I charge it? I take the cover off, pull the battery, and charge it with another lipo
charger. I had wanted to mount a USB port on the side that ran back to the mcu, but the
one I had wouldn't fit in the case when plugged into the board. So I gave up and just take it
out. The 500mAH batter lasts for many rowing sessions though, so I do it like once a month.
