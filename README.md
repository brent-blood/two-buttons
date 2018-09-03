# Two Buttons
Use two buttons to do simple things when your hands are occupied.

I have a rowing machine and I should use it more than I do. I have a television mounted where I can see it
with a Tivo Mini, and sometimes I watch television while I exercise - and I want to skip commercials but
I don't want to stop rowing to reach for the remote constantly. Sometimes I would rather read a book
while I row, but that's even more difficult. So I got to thinking... why not cobble together something
that can simplify it down to two buttons (_prev_ and _next_) attached to inputs on a microcontroller with
wifi - and sends signals to various things like skipping commercials on my Tivo or changing the page on 
an eBook - that I can access while rowing without stopping or taking my hands off the rower?

So I made that. It actually does make the time go faster.

## Components

### ESP32
I used an [Adafruit Feather Huzzah32](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/overview)
for the final solution, but I prototyped with a cheap NodeMCU board (ESP8266). I made it battery powered,
which would have worked with the NodeMCU and a 9V battery, but I chose to use a lipo battery and the
adafruit board has a builtin lipo charger, more inputs, etc. Use whatever you want really.

I combined this with the [Assembled Terminal Block](https://www.adafruit.com/product/2926) Feather addon
because it allows me to put it all together without having to solder the ESP32 to a board. I tend to tear
my creations apart eventually, and once the mcu is soldered to something, that gets messy - and this avoids
that pain.

### SD Card Reader
I also combined this with an SD card reader. In particular, I used the [Data Logger Featherwing](https://www.adafruit.com/product/2922)
and use this for storing configuration data like the details for accessing the wireless network, the
IP and port for controlling the Kindle, the Tivo's details, etc. It's overkill as I could have just hard
coded this stuff in the code (and in the first version of this I did just that) but it bothered me!
Maybe in a later version I'll move more stuff there such as GPIO mappings or provide an abstraction
for protocol communications that I then provide in a generic way via a configuration.

I don't use the RTC for anything at the moment. It's there if I ever want it I guess.

### Push Buttons
I don't even know where I got these little buttons, but if you're reading this and want to do this, you
probably know where to get them already. Look at the pictures if you want to see them.

### Proto Board
I think I have [these](https://www.amazon.com/Double-Sided-Board-Prototype-Paxcoo/dp/B01N3161JP/). I use
a small thin one for the button board that I actually interact with. If you don't use the terminal block
setup, this'll come in handy for connecting to the mcu's GPIOs as well.

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

### Potentiometer
I needed to pick from 3 functions: Kindle, Tivo 30s skip, Tivo commercial skip based on what I was doing
or what I was watching (some programs let you skip entire commercial breaks with one button push, but not
all (such as sporting events)), so a regular two-state switch wouldn't work - and I didn't want to mess with
something like a rotary encoder - so a pot connected to an analog input with different ranges assigned to
the different functions is easy. I don't even know what value I used as it realy doesn't matter. Maybe 10k?

### A Mac with Kindle App and Content to Read
I really should spend some more time with this to get it working with something other than my mac laptop
but I'm lazy (hence needing something to read while exercising) and this was easy due to Applescript
being so simple to use and the presence of a well-written Amazon Kindle app with lots of content. I
could probably get this working with a Linux-powered arm SoC too via something like [xdotool](https://github.com/jordansissel/xdotool)
but for now this is what I got.

### Tivo Mini
I was lucky in that I already was using this, and Tivo provides a really trivial [API](https://www.tivo.com/assets/images/abouttivo/resources/downloads/brochures/TiVo_TCP_Network_Remote_Control_Protocol.pdf)
for simulating presses on the remote control.

## Code

### two-buttons.cpp
This is my arduino sketch that polls for button pushes and sends UDP datagrams to the computer system
that is showing my book. You'll need to either modify it to have hard coded info for what I read
off the SD card, or provide an SD card with a proper config file on it - or modify this to read/write
from the mcu's EEPROM.

The SD card config file should be named "two-buttons.cfg", and an example config file looks like:

WIFI_AP=my_wifi_network
WIFI_PW=my_wifi_secret
KINDLE_HOST=192.0.2.10
KINDLE_PORT=9876
TIVO_HOST=192.0.2.11
TIVO_PORT=31339
DEBOUNCE_DELAY=500
DEBUG=0

You shouldn't need to supply everything if you don't use it, but I would anyways. I'll admit that
I didn't test this that thoroughly - so who knows - maybe it'll go crazy.

### two-button-reader.py
This is the client that runs on my mac. It listens for UDP datagrams that contain "prev" and "next"
and then runs a short little AppleScript to send the keyboard events for left and right arrow to the
Kindle app. It's absurd how easy Apple made this sort of thing! I went from having never used
AppleScript in my life to having working code in about a minute, most of which was spent using
Google. Job well done Apple.

## Assembly
Check out the fritzing diagram. Assembly is pretty easy to get it functional on the bench... though
making it pretty is always a challenge. Seriously, this is about as easy of a circuit as you'll
ever find - no resistors, capacitors, etc - just wires, pins, and solder. Good thing I have a
college degree in electrical engineering.

## Questions You May Have (comments I don't care to explain again)

**What did your wife say when she saw this?** The first words out of my wife's mouth were: "It looks
like a bomb".

**Do you actually use this?** Yes, I do. I bought a thousand dollar professional rowing machine and
attached a battery powered microcontroller to it with wires hanging out and I use it. Even though
it does look like a bomb.

**What about security?** The network connection is not authenticated, and it's not encrypted. I'm using
it in my friggin basement on my own wifi and that's fine. If you use this in public you are crazy,
but it's still probably fine to not have it authenticated or encrypted because seriously nobody
is going to mess with it - but in the spirit of hacking, feel free to extend it.

Seriously though, if you're actually asking this, be practical. I have a lot of respect for real
security experts because doing things right is actually really hard, but you have got to understand
when some stuff just doesn't matter.

**Why didn't you use bluetooth or xbee or smoke signals or something else?** I dunno. This is what I had
lying around and could get working quickly.

**Why is the arduino sketch file named two-buttons.cpp and not something that ends in .ino? Why
does it include a header file for Arduino.h?** Because the arduino IDE is an awful piece of 
software and I made this using [PlatformIO](https://platformio.org/). The file was actually
called main.cpp when I copied it off so consider this an improvement. You're welcome.

**Wow, calling the Arduino IDE an awful piece of software is pretty harsh, why do you think that?**
Let me walk that back a little bit - the Arduino IDE is functional, and it gets people up and
running with microcontrollers really quickly, so I shouldn't give it such a hard time. But I
imagine that anyone doing anything serious gets tired of it pretty quickly. It's written in Java
and so has all the downsides of that environment, it cannot be driven by the command line so
integrating it into a makefile is right out, and lastly (and this is huge) it doesn't support vi
key bindings. And that's just what I can think of off the top of my head!

**Did you test this?** Hardly at all.

**How long did this take to make?** I prototyped it in about half an hour on a breadboard. Then it
sat on my bench for about 3 weeks before I busted out the soldering iron and drill and made it work
for real. It only supported Kindle at that point - then I discovered the Tivo Remote API and hacked
out support for that along with the potentiometer switch in a couple hours and moved it all to a
bigger case.

I probably should have spent the time actually rowing rather than coding and soldering, but whatever.

**How do you charge it?** I used to take the cover off, pull the battery, and charge it with another
lipo charger. But since I moved from ESP8266 to ESP32 I put it into a larger case and exposed
the USB port externally - so now I can charge it without taking it apart. I tend to use the rower
before I lift weights, and so I now charge it while I'm lifting immediately after using it and then
unplug it when I'm done so it's not left plugged in indefinitely. That seems long enough to top
off the battery before I use it again.

**Do you have a website?** Of course I do. It's right [here](https://www.b-rent.com/).
