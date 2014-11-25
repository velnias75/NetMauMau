NetMauMau
=========

Server for the popular card game Mau Mau.

Setting up the build environment
--------------------------------

After checkout run `autoreconf -ifv` to set up the build environment.

`./configure && make`

builds the projects and

`make install`

installs it.

Client
------

A proof of concept Qt client can be found at [https://github.com/velnias75/NetMauMau-Qt-Client](https://github.com/velnias75/NetMauMau-Qt-Client)


Binary releases
===============

Ubuntu
------
Binary packages are available for Precise, Trusty, Utopic and Vivid
in my Launchpad PPA at [https://launchpad.net/~velnias/+archive/ubuntu/velnias](https://launchpad.net/~velnias/+archive/ubuntu/velnias)

Add the repository to your system: 

`sudo add-apt-repository ppa:velnias/velnias`

Debian 7
--------
[http://download.opensuse.org/repositories/home:/velnias/Debian_7.0](http://download.opensuse.org/repositories/home:/velnias/Debian_7.0)


Windows
-------
[https://sourceforge.net/projects/netmaumau/](https://sourceforge.net/projects/netmaumau/)


Rules
=====

Following are the rules currently implemented in NetMauMau:

General rules
-------------

At the beginning every player gets five random cards out of a set of 32 cards. Of the remaining 
cards one card's front is visible to all players and the rest serves as pool of cards to take if
a player cannot play a card or if the player has to take cards due to the *Seven* rule. If all 
pool cards are either in players hands or played out all cards except the visible card are 
shuffled again and made available as pool again.

The aim of NetMauMau is to get away all cards as fast as possible. To achieve this a player
can play out any card of either the same rank or the same suit. If a player cannot play out
any card the player has to take one from the pool and to suspend. Some cards trigger specific 
actions as described below.

Specific card rules
-------------------

All rules apply also to the visible card at the beginning of the game for the first player.

* **Seven**

   if a *Seven* is played out than the next player has either to take two more cards or play 
   out another *Seven*. In that case the next player has either to take plus two (i.e. four)
   cards or can also play out a *Seven* and so forth. At maximum one player has to take eight 
   cards if a sequence of four *Seven* are played out

* **Eight**

   if an *Eight* is played out, the next player has to suspend and the next player can play 
   a card. The player has **not** to take an extra card. An *Eight* played before takes 
   precedence, i.e. even if the next player has an *Eight*, the player has to suspend

* **Nine**

   there is no special rule for that rank

* **Ten**

   there is no special rule for that rank

* **Queen**

   there is no special rule for that rank

* **King**

   there is no special rule for that rank

* **Ace**

   there is no special rule for that rank

* **Jack**

   if a *Jack* of any suite is played out, the player can wish a new suit. A *Jack* can get
   played over any card except another *Jack*. An *Eight* played before takes precedence, i.e.
   even if the next player has a *Jack*, the player has to suspend
