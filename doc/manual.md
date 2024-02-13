
	= KeySeq
	
	KeySeq ist ein Sequenzer, der über (MIDI->CV) Keyboard Sequenzen erfassen kann 
	und aber auch über Keyboard gesteuert werden kann.
	
	== Features
	
	
	- 12 Sequenzen mit maximal 256 Steps.
	- Erfassen von Noten, verbundenen Noten und Pausen
	- Transpose Funktion
	
	== Grundprinzip
	

	Folgende Sequenzen sind in KEY-SEQ vorhanden:
	
	12 Record-Sequenzen
	 1 Play-Sequenz
	 1 Transpose-Sequenz
	
	Die Record-Sequenzen werden über (MIDI->CV) Keyboard erfasst. 
	Über die Funktion Play kann die aktuelle Record-Sequenz in die Play-Sequenz kopiert werden.
	Diese wird dann abgespielt.
	
	Über die Funktion Sel kann eine andere der 12 Record-Sequenzen ausgewählt werden, verändert werden und dann
	mit Play wieder in die Play-Sequenz kopiert werden. Der letzen Sequenz (Nummer 11) kommt eine besondere
	Bedeutung zu. Sie wird nicht in die Play-Sequenz kopiert, sondern in die Trans-Sequenz. Die Trans-Sequenz bestimmt
	die Basisnote der Play-Sequenz zum Transponieren. Nach jedem Durchlauf der Play-Sequenz, wird die nächste Note
	Trans-Sequenz ausgewählt.
	
	Die Steuerung der Eingabe findet über 
	
	- das virtuelle Keyboard des Moduls 
	- oder das angeschlossene (MIDI->CV) Keyboard 
	- oder über ein zweites angeschlossenes angeschlossene (MIDI->CV) Keyboard
	
	statt.
	
	== Verwendung
	
	
	Die Eingänge Run, Clk, Rst sind mit einem Clock Modul zu verbinden, das den Takt
	liefert und über das der Sequenzer gestartet und zurückgesetzt werden kann. Das Starten und
	Zurücksetzen ist auch mit den entsprechenden Schaltern des Moduls möglich. 
	Die Eingänge Run und Rst sind somit optional.
	
	An den Ausgängen CV, Gate und Vel wird die Sequenz ausgegeben. Über den Regler Gate, kann
	die Länge des Gate-Signals eingestellt werden.
	
	Die Eingänge CV, Gate und Vel sind mit einem MIDI->CV Modul zu verbinden oder mir einem Modul, 
	das ähnliche Signale liefert. Sowohl der Eingang als auch der Ausgang Vel ist optional.
	Über das angeschlossene Keyboard kann eine Sequenz eingegeben werden.
	
	Die Steuerung der Eingabe findet über das virtuelle Keyboard des Moduls statt. Folgende Befehle stehen zur
	Verfügung:
	
	Play - kopiert die aktuelle Record-Sequenz in die Play-Sequenz
	Prev - positioniert die Eingabe einen Schritt nach vorne
	Next - positioniert die Eingabe einen Schritt nach hinten
	Rest - erfasst eine Pause in der Sequenz
	Tie  - erfasst eine Note, die mit der vorherigen verbunden wird
	Del  - löscht die aktuelle Note
	Clear - löscht die Record-Sequenz
	Ins   - schaltet den Insert-Modus ein und aus: Noten werden eingefügt und nicht überschrieben.
	Sel   - Wählt eine andere Record-Sequenz aus. Jeder Taste des virtuellen Keyboard ist eine von 12 Sequenzen zugeordnet.
	        Zur Auswahl ist nach Sel die entsprechende Taste zu drücken.
	Copy  - Kopiert die aktuelle Record-Sequenz in eine andere Record-Sequenz. 
	        Die Zielsequenz ist danach auf dem virtuellen Keyboard auszuwählen (wie bei Sel).		
	Trans - schaltet den Transpose Modus ein und aus: die Note vom Keyboard wird nicht in der Sequenz erfasst 
	        und dient stattdessen zum Transponieren der Play-Sequenz.
	
	== Steuerung über Keyboard
	
	
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
	
	== Bedeutung der Lichter
	
	
	Play - Ein Kopieren der Sequenz in die Play-Sequenz wurde angefordert. Das kopieren findet aber erst statt,
	       wenn die aktuelle Sequenz zu ende gespielt wurde. So lange leuchtet die Play-Leuchte.
		   
	Clear - leuchtet so lange, wie eine Sequenz leer ist.
	
	Ins - zeigt an, das der Insert-Modus angeschaltet ist.
	
	Trans - zeigt an, das der Transpose-Modus angeschaltet ist.
	

	== Auswahl der Sequenz über CV
	
	
	Die zu Play-Sequenz kann auch über den Eingang Play gesteuert werden. 
	Sobald hier eine andere Spannung anliegt, wird die zum Notenwert passen Record-Sequenz in die 
	Play-Sequenz kopiert.

	== Transponieren der Play-Sequenz
	
	
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
	
	
	== Anzeige
	
	
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
	(die den selben Notenwert haben, wie der Vorgänger) werden durch "~~~" dargestellt.
	Das Ende der Sequenz wird mit einem " | " markiert.
	
	
	== Optionen
	
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
	
