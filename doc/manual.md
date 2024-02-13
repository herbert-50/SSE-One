# KEY-SEQ

## Features

- 12 sequences with a maximum of 256 steps.
- Capturing notes, tied notes and rests
- Transpose function for sequences

## Basic principle

The following sequences are present in KEY-Record sequences

12 Record sequences
 1 Play sequences
 1 Transpose sequences

The *record sequences* are recorded from the keyboard. Pressung a key will add or alter a step of the sequence.
The current *record sequence* can be copied into the *play sequence* using the **Play** function.
This sequence will then be played.

Using the **Sel** function, another of the 12 *record sequences* can be selected, changed with the keyboard and then
can be copied into the *play sequence* with **Play**. The last sequence (number 11) is special
meaning too. It is not copied into the *play sequence*, but into the *trans sequence*. The *trans sequence* determines
the base note of the *play sequence* to transpose. After each pass through the *play sequence* the next note
of *trans sequence* will be used.


The following options are available for controlling the sequences:

- the module's virtual keyboard
- or the connected (MIDI->CV) keyboard
- or a second connected (MIDI->CV) keyboard

## Usage

The *Run*, *Clk*, *Rst* inputs must be connected to a clock module that supplies the clock
and via which the sequencer can be started and reset. Starting and resetting
can also be started and reset using the corresponding switches on the module. 
The Run and Rst inputs are therefore optional.

The sequence is output at the *CV*, *Gate* and *Vel* outputs. The *Gate* control can be used to
the set the length of the gate signal .

The *CV*, *Gate* and *Vel* inputs must be connected to a *MIDI->CV* module or to a module
which provides similar signals. 

Both input and output *Vel* are optional.

Now a sequence can be entered using the connected keyboard.


The functions are started via the module’s virtual keyboard. The following commands are available:

	- **Play** : copies the current record sequence to the play sequence.
	- **Prev** : moves the input position of the current record sequence on step forward.
	- **Next** : moves the input position of the current record sequence on step backward.
	- **Rest** : adds a rest to the current record sequence.
	- **Tie**  : adds a tied note to the current record sequence.
	- **Del**  : deletes a note from the current record sequence.
	- **Clear** : deletes all notes from the current record sequence.
	- **Ins**   : switches the insert mode on and off: notes will be inserted or overwritten.
	- **Sel**   : starts selecting an other record sequence. Every following key of the virtual keyboard stands for one of 12 sequences.
			To select it, the key has to be pressed.
	- **Copy**  : Copies the current record sequence into an other record sequence. 
			The target sequence has to be selected with a key (same as *Sel*).		
	- **Trans** : switches the transpose mode on and off: the note from the keyboard is not recorded in the sequence
			and is used instead to transpose the play sequence.

## Steuerung über Keyboard


Die Befehle des virtuellen Keyboards können auch mit dem (MIDI->CV) Keyboard ausgelöst werden.
Dazu ist die enstprechende Taste zu drücken und ohne diese loszulassen eine zweite Taste. 
Also beispielsweise die Taste C4 drücken und festhalten. Dann D4 drücken und beide Tasten loslassen, 
um den Befehl Play auszulösen. Die zweite Taste kann eine beliebige sein. Die Oktave spielt nur bei den
Befehlen Prev und Next eine Rolle und ist ansonsten egal. Prev oder Next in einer Octave < 4 bewegt 
die Eingabeposition um einen Schritt. Prev oder Next in einer Octave > 4 bewegt 
die Eingabeposition um einen 11 Schritte (=eine Zeile) und in Oktave 4 sind es 5 Schritte.

Alternativ kann auch ein zweites Keyboard (zB Launchpad) an die unteren CV Gate Eingänge angeschlossen werden,
um die Befehle auszulösen. In diesem Fall ist keine zweite Taste zu drücken. Über das erste Keyboard kann dann die Sequenz
eingegeben werden. Das zweite dient zum Auslösen der Befehle.

## Bedeutung der Lichter


Play - Ein Kopieren der Sequenz in die Play-Sequenz wurde angefordert. Das kopieren findet aber erst statt,
	   wenn die aktuelle Sequenz zu ende gespielt wurde. So lange leuchtet die Play-Leuchte.
	   
Clear - leuchtet so lange, wie eine Sequenz leer ist.

Ins - zeigt an, das der Insert-Modus angeschaltet ist.

Trans - zeigt an, das der Transpose-Modus angeschaltet ist.


## Auswahl der Sequenz über CV
	
Die zu Play-Sequenz kann auch über den Eingang Play gesteuert werden. 
Sobald hier eine andere Spannung anliegt, wird die zum Notenwert passen Record-Sequenz in die 
Play-Sequenz kopiert.

## Transponieren der Play-Sequenz

Hierzu gibt es drei Möglichkeiten, die verschiedene Prioritäten haben. Welche Möglichkeit gerade
verwendet wird läßt sich an der Farbe erkennen, in der die Basisnote zum Transponieren im Trans-Diplay 
dargestellt wird.

Die höchste Priorität hat der Transpose-Modus, der über die Funktion Trans ein und ausgeschaltet wird.
Ist er eingeschaltet, dann bestimmt er anhand der über das Keyboard eingegeben Note die Basisnote zum Transponieren.
Die Note wird im Trans-Display nach dem Tastendruck in gelb und dann in blau angezeigt, sobald sie aktiviert wird.
Die anderen Möglichkeiten werden nur berücksichtigt, wenn der Transpose-Modus ausgeschaltet ist.

Die zweit höchste Priorität hat der Eingang Trans. Bei der Änderung einer Spannung wird die daraus resultierende Note
(gemäß 1V/Oct) als Basisnote zum Transponieren eingestellt. Die Note wird im Trans-Display nach der Spannungsänderung
in gelb und dann in grün angezeigt, sobald sie aktiviert wird.

Ist weder der Transpose-Modus an, noch der Eingang Trans belegt, dann wird die Basisnote zum Transponieren über die 
Transpose-Sequenz bestimmt. Sie kann befüllt werden, indem die Record-Sequenz 11 mit der Play Funktion kopiert 
wird (Autoplay funktioniert hier nicht).
Damit kommt der Record-Sequenz 11 eine besondere Bedeutung zu. Sie wird im Display deshalb in lila dargestellt.
Auf im Trans-Display wird die aktuelle Basisnote in lila dargestellt.

Ist die Trans-Sequenz leer, dann wird C4 als Basisnote zum Transponieren verwendet und im Display weiß dargestellt.


## Anzeige


Gelbe Anzeigen beziehen sich auf die Record-Sequenz und grüne auf die Play Sequenz.

Die großen Anzeigen links und rechts zeigen den aktuellen Schritt an. 
Die kleinen Anzeigen daneben zeigen die Gesamtanzahl Schritte der Sequenz an. Die dritte Anzeige zeigt die Nummer
der Sequenz (0-11) an.

In der großen dreizeiligen Anzeige darunter wird die aktuelle Record Sequenz (oder ein Teil davon je nach Länge)
ausgegeben. Die aktuelle Position befindet sich immer in der Mitte. 
Das bedeutet bei den Befehlen Next und Prev wird nicht die Position in der Anzeige verschoben, sondern die angezeigten Noten.
Somit sind immer 16 Steps vor und 16 Steps hinter der aktuellen Position sichtbar.

Notenwerte werden durch einen Buchstaben für den Notenwert und eine Zahl für die Oktave dargestellt. 
Pausen werden durch das Zeichen " ' " dargestellt und verbundene Noten 
(die den selben Notenwert haben, wie der Vorgänger) werden durch "\~\~\~" dargestellt.
Das Ende der Sequenz wird mit einem " | " markiert.


## Optionen

... werden über das Menü eingestellt.

Cmd from keyboard : legt fest, ob eine (Zweitasten) Steuerung über das erste Keyboard möglich ist.
					Bei Verwendung eines zweiten Keyboard oder des virtuellen Keyboard kann diese Option ausgeschaltet werden.
					
Cmd from last key : wenn gesetzt, dann bestimmt nicht die zuerst gedrückte Taste den Befehl, sondern die die
					als zweites gedrückt wurde. C4 D4 würde dann statt Play Rest auslösen.
					
Autoplay when
... new note      : löst den Befehl Play aus, wenn eine (neue) Note in der Record-Sequenz eingegeben wurde.
... new rest      : löst den Befehl Play aus, wenn eine Pause in der Record-Sequenz eingegeben wurde.
... new tie       : löst den Befehl Play aus, wenn eine verbundene Note in der Record-Sequenz eingegeben wurde.
... delete        : löst den Befehl Play aus, wenn ein Step in der Record-Sequenz gelöscht wurde.
... select sequence : löst den Befehl Play aus, wenn eine andere Record-Sequenz gewählt wurde.

Play CV 0..10V    : Setzt den Bereich für den Eingang Play zur Auswahl der Sequenz auf 0-10V statt auf C-B.

Play after end of Sequenz: Das Kopieren einer Record-Sequenz in die Play-Sequenz erfolgt erst am 
						   Ende der aktuellen Sequenz. Wenn die Option nicht gesetzt ist, erfolgt das Kopieren
						   mit der Clock.

Transpose after end of Sequenz: Die Basisnote zum Transponieren wird erfasst und wirkt sich aber erst am 
						   Ende der aktuellen Sequenz aus. Wenn die Option nicht gesetzt ist, wird die Basisnote
						   mit der Clock gesetzt.

Set Seqence after copy: Nach dem Kopieren wird die Ziel Sequenz aktiviert.

