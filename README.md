Build instructions, binaries and rules
======================================

Prerequisites
-------------

* [GNU GCC compiler](https://gcc.gnu.org/) (currently NetMauMau uses a certain amount of features
  exclusive to GCC)
* [POPT library](http://rpm5.org/files/popt/) >= 1.10
* xxd (from the vim package [on **Debian** based distributions *vim-common*; on **Gentoo** 
  *vim-core*])
* **gawk** (`mawk` will cause the build to fail)
* libmagic (*optional, but recommended*)
* [GNU Scientific Library](http://www.gnu.org/software/gsl/) >= 1.9 (*optional*, for better 
  random number generator)
* [libmicrohttpd](http://www.gnu.org/software/libmicrohttpd) (*optional, for the integrated 
  webserver*)
* [zlib](http://www.zlib.net/) (*optional, for compressed webserver responses*)
* [sqlite3](https://www.sqlite.org/)
* [libtool](http://www.gnu.org/software/libtool/)
* [Lua](http://www.lua.org/) 5.1

(x)inetd support
----------------

On *Linux/BSD systems* you can let start the server on demand as well, by adding it as service to 
**(x)inetd**.

A *sample* **xinetd** *service configuration* can look as following:

    service netmaumau
    {
	    disable = no
	    per_source = 1
	    port = 8899
	    socket_type = stream
	    protocol = tcp
	    user = root
	    server = /usr/bin/nmm-server
	    server_args = --inetd
	    type = UNLISTED
	    wait = yes
	    instances = 1
    }

Windows service
---------------

If you want to start the *NetMauMau server* at system start, you can install the *NetMauMau server* 
as service. I recommend following workflow:

 * Download [NSSM](http://nssm.cc/)
 * In the console (`cmd.exe`) run `nssm install`
 * Choose `nmm-server.exe` and add the arguments as you like :)

Before updating *NetMauMau* you'll need to stop the service if running: `sc stop <ServiceName>` 
or `services.msc`


Setting up the build environment
--------------------------------

After checkout run `autoreconf -ifv` to set up the build environment.

`./configure && make` builds the projects and `make install` installs it.
See `configure --help` for more options to customize and general help.

Client
------

A Qt client can be found at
[https://github.com/velnias75/NetMauMau-Qt-Client](https://github.com/velnias75/NetMauMau-Qt-Client)


Binary releases
===============
[![Download NetMauMau](https://img.shields.io/sourceforge/dm/netmaumau.svg)]
(https://sourceforge.net/projects/netmaumau/files/latest/download)

Gentoo
------
NetMauMau is available on Gentoo via the overlay **games-overlay** which
can be added by `layman`
The GitHub repository of **games-overlay** is here:
[https://github.com/hasufell/games-overlay](https://github.com/hasufell/games-overlay)

**Adding the overlay**

With paludis: see 
[Paludis repository configuration](http://paludis.exherbo.org/configuration/repositories/index.html)

With layman:
```layman -f -o https://raw.github.com/hasufell/games-overlay/master/repository.xml -a games-overlay```
or ```layman -a games-overlay```

Install NetMauMau with `emerge games-server/netmaumau`

Ubuntu
------
Binary packages are available for Precise, Trusty, Utopic and Vivid in my Launchpad PPA at
[https://launchpad.net/~velnias/+archive/ubuntu/velnias](https://launchpad.net/~velnias/+archive/ubuntu/velnias)

Add the repository to your system:

`sudo add-apt-repository ppa:velnias/velnias`

PlayDeb.net offers [binaries](http://www.playdeb.net/game/NetMauMau) too.

Debian 7/8
--------
* insert following line into `/etc/apt/sources.list` (replace `V` with `7` or `8` respectively)

    `deb http://download.opensuse.org/repositories/home:/velnias/Debian_V.0 /`

* *(optionally)* add the GPG-key

    `wget -O- http://sourceforge.net/projects/netmaumau/files/debian-apt.asc | apt-key add -`

* run `apt-get update` 

Arch Linux
--------
The package is available in the AUR at [https://aur.archlinux.org/packages/netmaumau-server/](https://aur.archlinux.org/packages/netmaumau-server/)

* run `yaourt -S netmaumau-server netmaumau-client` 

openSUSE
--------
* http://software.opensuse.org/download.html?project=home%3Aecsos&package=NetMauMau-server

Windows
-------
[![Download NetMauMau](https://a.fsdn.com/con/app/sf-download-button)]
(https://sourceforge.net/projects/netmaumau/files/latest/download)

Rules
=====

Following are the rules currently implemented in NetMauMau:

General rules
-------------

At the beginning every player gets five random cards out of a set of 32
cards. Of the remaining cards one card's front is visible to all players and
the rest serves as pool of cards to take if a player cannot play a card or
if the player has to take cards due to the *Seven* rule. If all pool cards
are either in players hands or played out all cards except the visible card
are shuffled again and made available as pool again.

The aim of NetMauMau is to get away all cards as fast as possible. To achieve
this a player can play out any card of either the same rank or the same
suit. If a player cannot play out any card the player has to take one from the
pool and to suspend. Some cards trigger specific actions as described below.

If a player has lost the points of the player's cards are summed up. If the
player has lost due a played out Jack, the points are doubled. The higher
that value the worse the game is lost.

Specific card rules
-------------------

All rules apply also to the visible card at the beginning of the game for
the first player.

* **Seven** (1 point)

   if a *Seven* is played out than the next player has either to take two
   more cards or play out another *Seven*. In that case the next player has
   either to take plus two (i.e. four) cards or can also play out a *Seven*
   and so forth. At maximum one player has to take eight cards if a sequence
   of four *Seven* are played out. After a player took the cards, but cannot
   play any card, the player has **not** to take a extra card

* **Eight** (2 points)

   if an *Eight* is played out, the next player has to suspend and the next
   player can play a card. The player has **not** to take an extra card. An
   *Eight* played before takes precedence, i.e. even if the next player has
   an *Eight*, the player has to suspend

* **Nine** (3 points)

   if more than two players joined the game and enabled with `--direction-change|-d`,
   than a change of direction is performed. If the player count drops to two, this card 
   get the same meaning like *Eight*. Else there is no special rule for that rank

* **Ten** (4 points)

   there is no special rule for that rank

* **Queen** (5 points)

   the same as for **Ace rounds** but with *Queen* if enabled with `--ace-round=QUEEN|-aq`

* **King** (6 points)

   the same as for **Ace rounds** but with *King* if enabled with `--ace-round=KING|-ak`

* **Ace** (11 points)

   if the **Ace rounds** are enabled (`--ace-round=ACE|-aa`), a player can, if there
   are two or more *Aces* on hand, start an **Ace round**. Within the 
   **Ace round**  other players can only play out *Aces* or have to suspend and 
   take a card. The player can stop the **Ace round** at any time. The 
   **Ace round** is also stopped if the player plays the last *Ace*.
   Otherwise *Ace* has no special rule.

* **Jack** (20 points)

   if a *Jack* of any suite is played out, the player can wish a new suit. A
   *Jack* can get played over any card except another *Jack*. An *Eight*
   played before takes precedence, i.e. even if the next player has a *Jack*,
   the player has to suspend
