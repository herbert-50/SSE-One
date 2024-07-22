#include "plugin.hpp"

// Uncomment to activate debug messages
#define DEBUG(format, ...) 

#define MAX_STEPS 256
#define NUM_SEQ 12
#define EMPTY_VOLTAGE -99.0f

#define GATEMODE_EOS 0
#define GATEMODE_NORMAL 1
#define GATEMODE_REST 2
#define GATEMODE_TIED 3

#define DISPLAYNOTES 33
#define DISPLAYNOTES_PER_LINE 11
#define DISPLAYNOTE_ACT 16

namespace pluginSSE {

int compareCV(float cv1, float cv2) {
	float diff = cv1 - cv2;
	if (fabs(diff) <= 0.005f) {
		return 0;
	}
	return (diff > 0 ? 1 : -1);
}

void fillNoteFromVoltage(char* keyName, float cvVoltage) {
	strcpy(keyName, "ER");
	int octVoltage = (int)floor(cvVoltage); 
	float noteVoltage = cvVoltage - octVoltage; 
	char octName[12];
	sprintf(octName, "%d", octVoltage+4);
	//DEBUG("CV=%f oct=%s - %d note=%f", cvVoltage, octName, octVoltage, noteVoltage);
	if (compareCV(noteVoltage, 0.000000f) == 0) { //C
		strcpy(keyName, "C");
	}
	else if (compareCV(noteVoltage, 0.083333f) == 0) { //C#
		strcpy(keyName, "C#");
	}
	else if (compareCV(noteVoltage, 0.166667f) == 0) { //D
		strcpy(keyName, "D");
	}
	else if (compareCV(noteVoltage, 0.250000f) == 0) { //D#
		strcpy(keyName, "D#");
	}
	else if (compareCV(noteVoltage, 0.333333f) == 0) { //E
		strcpy(keyName, "E");
	}
	else if (compareCV(noteVoltage, 0.416667f) == 0) { //F
		strcpy(keyName, "F");
	}
	else if (compareCV(noteVoltage, 0.500000f) == 0) { //F#
		strcpy(keyName, "F#");
	}
	else if (compareCV(noteVoltage, 0.583333f) == 0) { //G
		strcpy(keyName, "G");
	}
	else if (compareCV(noteVoltage, 0.666667f) == 0) { //G#
		strcpy(keyName, "G#");
	}
	else if (compareCV(noteVoltage, 0.750000f) == 0) { //A
		strcpy(keyName, "A");
	}
	else if (compareCV(noteVoltage, 0.833333f) == 0) { //A#
		strcpy(keyName, "A#");
	}
	else if (compareCV(noteVoltage, 0.916667f) == 0) { //B
		strcpy(keyName, "B");
	}
	strcat(keyName, octName);
	//DEBUG("keyname= <%s>", keyName);
}	


float noteNameToVoltage(char* keyName) {
	float noteVoltage = 0.f;
	int octVoltage = 0;
	
	switch (*(keyName)) {
		case 'C': noteVoltage = 0.000000f; break;
		case 'D': noteVoltage = 0.166667f; break;
		case 'E': noteVoltage = 0.333333f; break;
		case 'F': noteVoltage = 0.416667f; break;
		case 'G': noteVoltage = 0.583333f; break;
		case 'A': noteVoltage = 0.750000f; break;
		case 'B': noteVoltage = 0.916667f; break;
	}
	if (*(keyName+1) == '#') {
		noteVoltage = noteVoltage + 0.0833333f;
		octVoltage = *(keyName+2) - '0' - 4;
	}
	else {
		octVoltage = *(keyName+1) - '0' - 4;
	}
	return (float)octVoltage + noteVoltage;
}

struct StepValue {
	float CV;
	int gatemode;
	float velocity;
};

struct Sequence {
	StepValue step[MAX_STEPS+1];
	int len;
	int pos;
	
	void dump(char* info) {
		DEBUG("dump seq %s pos=%d len=%d", info, pos, len);
		for (int i = 0; i < len; i++) {
			DEBUG("%s : %d CV=%f gate=%i", info, i, step[i].CV, step[i].gatemode);
		}
	}
	int getGatemode(int offset = 0) {
		if (pos + offset >= 0 && pos + offset <= MAX_STEPS)
			return step[pos + offset].gatemode;
		return -1;
	}
	
	void setGatemode(int gatemode, int offset = 0) {
		if (pos + offset >= 0 && pos + offset <= MAX_STEPS)
			step[pos + offset].gatemode = gatemode;
	}
	
	float getCV(int offset = 0) {
		if (pos + offset >= 0 && pos + offset <= MAX_STEPS)
			return step[pos + offset].CV;
		return EMPTY_VOLTAGE;
	}
	
	void setCV(float CV, int offset = 0) {
		if (pos + offset >= 0 && pos + offset <= MAX_STEPS)
			step[pos + offset].CV = CV;
	}
	
	float getVelocity(int offset = 0) {
		if (pos + offset >= 0 && pos + offset <= MAX_STEPS)
			return step[pos + offset].velocity;
		return 0.f;
	}
	
	void setVelocity(float velocity, int offset = 0) {
		if (pos + offset >= 0 && pos + offset <= MAX_STEPS)
			step[pos + offset].velocity = velocity;
	}
	
	bool insertStep(float CV, int gatemode, float velocity) {
		// make a gap
		if (len < MAX_STEPS) {
			/*if (pos < len) {
				pos++;
			}*/
			for (int i=len; i > pos; i--) {
				step[i].CV = step[i-1].CV;
				step[i].gatemode = step[i-1].gatemode;
				step[i].velocity = step[i-1].velocity;
			}
			
			step[pos].CV = CV;
			step[pos].gatemode = gatemode;
			step[pos].velocity = velocity;
			
			len++;
			step[len].gatemode = GATEMODE_EOS;
			return true;
		}
		return false;
	}

	bool setStep(float CV, int gatemode, float velocity) {
		if (step[pos].gatemode == GATEMODE_EOS) {
			if (len < MAX_STEPS) {
				len++;
				step[len].gatemode = GATEMODE_EOS;
			}
			else {
				return false;
			}
		}
		step[pos].CV = CV;
		step[pos].gatemode = gatemode;
		step[pos].velocity = velocity;
		return true;
	}

	bool deleteStep() {
		if (pos == 0 || len == 0) {
			return false;
		}
		if (step[pos].gatemode == GATEMODE_EOS) {
			// dont delete GATEMODE_EOS, delete step before in this case
			pos--;
		}
		for (int i=pos; i < len; i++) {
			DEBUG("delete step copy %d (to %f) %d (%f)", i+1, step[i+1].CV, i , step[i].CV);
			step[i].CV = step[i+1].CV;
			step[i].gatemode = step[i+1].gatemode;
			step[i].velocity = step[i+1].velocity;
		}
		len--;
		step[len].gatemode = GATEMODE_EOS;
		return true;
	}
	
	void reset() {
		pos = 0;
	}
	
	bool next(int steps = 1) {
		if (pos + steps <= len) {
			// enough space for steps
			pos = pos + steps;
			return true;
		}
		else if (pos != len) {
			// move to end
			pos = len;
			return true;
		}
		// already at the end
		return false;
	}

	bool prev(int steps = 1) {
		if (pos - steps >= 0) {
			// enough space for steps
			pos = pos - steps;
			return true;
		}
		else if (pos != 0) {
			// move to beginn
			pos = 0;
			return true;
		}
		// already at the begin
		return false;
	}
	
	void clear() {
		pos = 0;
		len = 0;
		step[pos].gatemode = GATEMODE_EOS;
	}

	bool isEmpty() {	
		return len == 0;
	}
	
	bool isFirstStep() {
		return pos == 0;
	}
	
	bool isAfterLastStep() {
		return step[pos].gatemode == GATEMODE_EOS;;
	}
		
	void copy(Sequence* otherSeq) {
		int i = 0;
		while(true) {
			int gatemode = otherSeq->step[i].gatemode;
			if (gatemode == GATEMODE_EOS) {
				step[i].gatemode = GATEMODE_EOS;
				break;
			}
			step[i].CV = otherSeq->step[i].CV;
			step[i].velocity = otherSeq->step[i].velocity;
			step[i].gatemode = gatemode;
			//DEBUG("Sequence.copy(): %d CV=%f gatemode=%d", i, otherSeq->step[i].CV, gatemode);
			i++;
		}
		len = i;
	}
	
	void fillNoteFromStep(char* noteName, int idx) {
		// DEBUG("Sequence.fillNoteFromStep(): %d CV=%f gate=%d",idx, step[idx].CV, step[idx].gatemode);
		switch (step[idx].gatemode) {
		case GATEMODE_NORMAL:
			fillNoteFromVoltage(noteName, step[idx].CV);
			break;
		case GATEMODE_REST:
			strcpy(noteName, "R ");
			break;
		case GATEMODE_TIED:
			strcpy(noteName, "T ");
			break;
		case GATEMODE_EOS:
			strcpy(noteName, "|");
			break;
		}
	}
	
	void createString(char* strBuf) {
		char noteName[5];
		
		strcpy(noteName, "");
		strcpy(strBuf, "");
		int p = 0;
		while (step[p].gatemode != GATEMODE_EOS) {
			fillNoteFromStep(noteName, p);
			strcat(strBuf, noteName);
			if (step[p].gatemode == GATEMODE_NORMAL) {
				if (step[p].velocity != 10.0f) {
					char strVel[12];
					sprintf(strVel, "_%d", (int)step[p].velocity*128/10);
					strcat(strBuf, strVel);
				}
				strcat(strBuf, " ");
			}
			p++;
		}
	}

	int createFromString(char* strBuf) {
		char* ptr;
		float noteVoltage = 0.00000f;
		int p = 0;
		int iVel = 0;
		
		ptr = strtok(strBuf, " ");
		while(ptr != NULL) {
			int len = strlen(ptr);
			switch (*(ptr)) {
			case 'R': 
				step[p].CV = EMPTY_VOLTAGE;
				step[p].gatemode = GATEMODE_REST;
				break;
			case 'T':	
				step[p].CV = noteVoltage;
				step[p].gatemode = GATEMODE_TIED;
				break;
			default:
				noteVoltage = noteNameToVoltage(ptr);
				step[p].CV = noteVoltage;
				step[p].gatemode = GATEMODE_NORMAL;
				break;
			}
			iVel = 128;
			if (len > 2 && *(ptr+2) == '_') {
				iVel = atoi(ptr+3);
			}
			else if (len > 3 && *(ptr+3) == '_') {
				iVel = atoi(ptr+4);
			}
			step[p].velocity = (iVel * 10.0) / 128.0;
			p++;
			ptr = strtok(NULL, " ");
		}
		step[p].CV = EMPTY_VOLTAGE;
		step[p].gatemode = GATEMODE_EOS;
		return p;
	}

};

struct KeySeq : Module {
	enum ParamId { TIECLICK_PARAM, GATELEN_PARAM, RUN_PARAM,
		RST_PARAM, NEXT_PARAM, PREV_PARAM, SQTRA_PARAM, COPY_PARAM, RES_PARAM, CLEAR_PARAM,
		PLAY_PARAM, TIE_PARAM, DEL_PARAM, INS_PARAM, REST_PARAM, SEL_PARAM, TRANS_PARAM, 
		RECORD_PARAM,
		PARAMS_LEN
	};
	enum InputId { CV_INPUT, RUN_INPUT, GATE_INPUT, CLK_INPUT, VEL_INPUT, RST_INPUT,
		PLAY_INPUT,	CV2_INPUT, GATE2_INPUT, TRANS_INPUT,
		INPUTS_LEN
	};
	enum OutputId {	CV_OUTPUT, GATE_OUTPUT, VEL_OUTPUT, END_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId { RUN_LIGHT, RST_LIGHT, ENUMS(PREV_LIGHT, 3), ENUMS(NEXT_LIGHT, 3), 
		ENUMS(CLEAR_LIGHT, 3), ENUMS(SQTRA_LIGHT, 3), ENUMS(COPY_LIGHT, 3),
		ENUMS(PLAY_LIGHT, 3), ENUMS(REST_LIGHT, 3), ENUMS(TIE_LIGHT, 3), ENUMS(DEL_LIGHT, 3), 
		ENUMS(INS_LIGHT, 3), ENUMS(TRANS_LIGHT, 3), ENUMS(SEL_LIGHT, 3),
		KEYPRESS_LIGHT, RECORD_LIGHT,
		LIGHTS_LEN
	};
	enum LightColor { OFF, YELLOW, GREEN, RED, BLUE, WHITE, ON };

	// control the workflow
	dsp::TSchmittTrigger<float>	clock;
	dsp::TSchmittTrigger<float>	run;
	dsp::TSchmittTrigger<float>	gatein;
	dsp::TSchmittTrigger<float>	gatein2;
	float gateStartCvVoltage;
	bool bWaitForCmd;
	bool bRecordNote;
	int cmdLight;
	int gatelen;
	int gatelenEOS;
	bool bWaitForSel;
	bool bWaitForCopy;
	bool bTransMode;
	
	std::atomic_flag thread_flag;
	dsp::ClockDivider lightDivider;
	
	// the sequences
	Sequence seqREC[NUM_SEQ];
	int seqRECIdx;
	Sequence* pSeqREC;
	
	Sequence seqPLAY;
	int seqPLAYIdx; // Just for display the last copied seqREC
	Sequence* pSeqPLAY;

	int seqRECToPLAYIdx;
	float lastCopyVoltage;

	Sequence seqTRANS;
	int seqTRANSIdx; // Just for display the last copied seqREC
	Sequence* pSeqTRANS;
	
	// common buffer for translating notes to text	
	char seqTextBuff[MAX_STEPS*10];

	const int keyLights[12] {PLAY_LIGHT, PREV_LIGHT, REST_LIGHT, NEXT_LIGHT, TIE_LIGHT, DEL_LIGHT, 
				                              CLEAR_LIGHT, INS_LIGHT, SQTRA_LIGHT, SEL_LIGHT, COPY_LIGHT, TRANS_LIGHT};

	int64_t lastFrame;

	// handling transpose
	char transKey[5];
	NVGcolor transKeyColor;
	float newTransCVVoltage;
	float transCVVoltage;
	float lastTransInputCVVoltage;

	// Move n steps forward on "next" or backward on "prev"
	int moveSteps;
	
	// Notes in display
	bool displayNeedsUpdate;
	char displayNotes[DISPLAYNOTES][4];
	NVGcolor seqNoteDiplayColor;
	// Save params on select sequence
	float saveINS_PARAM;
	float saveTRANS_PARAM;
	
	// Menuoptions
	bool bCmdFromLastKey;
	bool bKeyboardControl;
	bool bAutoPlayOnNewNote;
	bool bAutoPlayOnNewRest;
	bool bAutoPlayOnNewTie;
	bool bAutoPlayOnDelete;
	bool bAutoPlayOnSeqSelected;
	bool bAutoPlayAlways;
	bool bPlayUse10;
	bool bCopyOnEOS;
	bool bTransOnEOS;
	bool bSetSeqOnCopy;
	bool bSelectOnPlayInput;

	KeySeq() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
#ifdef SMART_TIE
		configParam(TIECLICK_PARAM, 100, 5000, 200, "Double click time (ms)");
#endif
		configParam(GATELEN_PARAM, 10, 1000, 250, "Length of gate signal");
		configInput(CV_INPUT, "1V/OCT");
		configInput(GATE2_INPUT, "Gate for commands");
		configInput(CV2_INPUT, "1V/OCT for commands");
		configInput(GATE_INPUT, "Gate");
		configInput(RUN_INPUT, "Run");
		configInput(CLK_INPUT, "Clock");
		configInput(VEL_INPUT, "Velocity");
		configInput(RST_INPUT, "Reset");
		configInput(PLAY_INPUT, "Copy record sequence to play sequence");
		configOutput(CV_OUTPUT, "1V/OCT");
		configOutput(GATE_OUTPUT, "Gate");
		configOutput(VEL_OUTPUT, "Velocity");
		configOutput(END_OUTPUT, "End of sequence");
		
		configSwitch(RUN_PARAM, 0.f, 1.f, 0.f, "Run sequence", {"Off", "Oon"});
		configSwitch(RST_PARAM, 0.f, 1.f, 0.f, "Reset", {"Off", "On"});
		configSwitch(NEXT_PARAM, 0.f, 1.f, 0.f, "Next step", {"Off", "On"});
		configSwitch(PREV_PARAM, 0.f, 1.f, 0.f, "Prev step", {"Off", "On"});
		configSwitch(SQTRA_PARAM, 0.f, 1.f, 0.f, "Reserved", {"Off", "On"});
		configSwitch(COPY_PARAM, 0.f, 1.f, 0.f, "Copy sequence", {"Off", "On"});
		configSwitch(CLEAR_PARAM, 0.f, 1.f, 0.f, "Clear sequence", {"Off", "On"});
		configSwitch(PLAY_PARAM, 0.f, 1.f, 0.f, "Copy record sequence to play sequence", {"Off", "On"});
		configSwitch(TIE_PARAM, 0.f, 1.f, 0.f, "Tied mode", {"Off", "On"});
		configSwitch(DEL_PARAM, 0.f, 1.f, 0.f, "Del note", {"Off", "On"});
		configSwitch(INS_PARAM, 0.f, 1.f, 0.f, "Insert mode", {"Off", "On"});
		configSwitch(REST_PARAM, 0.f, 1.f, 0.f, "Add a rest", {"Off", "On"});
		configSwitch(SEL_PARAM, 0.f, 1.f, 0.f, "Select sequence", {"Off", "On"});
		configSwitch(TRANS_PARAM, 0.f, 1.f, 0.f, "Transpose mode", {"Off", "On"});

		lightDivider.setDivision(16);
		thread_flag.clear();
		doReset();
	}
	
	void doReset() {
		gatelen = 0;
		gatelenEOS = 0;
		lastFrame = 0;
		bWaitForSel = false;
		bWaitForCopy = false;
		bTransMode = false;
		
		// create empty sequences		
		for (int i = 0; i < NUM_SEQ; i++) {
			seqREC[i].clear();
		}
		seqRECIdx = 0;
		pSeqREC = &(seqREC[seqRECIdx]);
		
		seqPLAY.clear();
		seqPLAYIdx = 0;
		pSeqPLAY = &(seqPLAY);
		
		seqRECToPLAYIdx = -1;
		lastCopyVoltage = 0.f;

		seqTRANS.clear();
		seqTRANSIdx = 0;
		pSeqTRANS = &(seqTRANS);
		
		clock.reset();
	
		moveSteps = 1;
		
		gatein.reset();
		gateStartCvVoltage = EMPTY_VOLTAGE;
		bWaitForCmd = false;
		bRecordNote = false;
		cmdLight = -1;
		newTransCVVoltage = 0.f;
		lastTransInputCVVoltage = 0.f;
		transCVVoltage = 0.f;
		fillNoteFromVoltage(transKey, transCVVoltage);
		transKeyColor = SCHEME_WHITE;
		
		bCmdFromLastKey = false;
		bAutoPlayAlways = false;
		bAutoPlayOnNewNote = false;
		bAutoPlayOnNewRest = false;
		bAutoPlayOnNewTie = false;
		bAutoPlayOnDelete = false;
		bAutoPlayOnSeqSelected = false;
		bCopyOnEOS = true;
		bTransOnEOS = true;
		bSetSeqOnCopy = true;
		bKeyboardControl = true;
		bPlayUse10 = false;
		bSelectOnPlayInput = false;

		params[RECORD_PARAM].setValue(1.f);
		
		seqNoteDiplayColor = SCHEME_YELLOW;
		updateDisplayNotes();
	}
	
	void onReset(const ResetEvent &e) override {
		Module::onReset(e);
		doReset();
	}

	void updateDisplayNotes() {
		char noteName[5];
		int middle = DISPLAYNOTE_ACT;
		int seqpos = 0;
		for (int i = 0; i < DISPLAYNOTES; i++) {
			seqpos = pSeqREC->pos - middle + i;
			if (seqpos < 0 || seqpos > pSeqREC->len) {
				strcpy(noteName, "   ");
			}
			else {
				pSeqREC->fillNoteFromStep(noteName, seqpos);
				if (*noteName == 'R') {
					strcpy(noteName, "'");
				}
				else if (*noteName == 'T') {
					strcpy(noteName, "~~~");
				}
				else if (strlen(noteName) < 3) {
					//strcat(noteName, " ");
				}
			}
			strcpy(displayNotes[i], noteName);
		}
	}

	void setLightColor(int id, LightColor color) {
		bool bRGB = false;
		for (int i = 0; i < 12; i++) {
			if (id == keyLights[i]) {
				bRGB = true;
				break;
			}
		}
		if (bRGB) {
			switch (color) {
			case OFF:
				lights[id + 0].setBrightness(0.f);
				lights[id + 1].setBrightness(0.f);
				lights[id + 2].setBrightness(0.f);
				break;
			case RED:
				lights[id + 0].setBrightness(1.f);
				lights[id + 1].setBrightness(0.f);
				lights[id + 2].setBrightness(0.f);
				break;
			case GREEN:
				lights[id + 0].setBrightness(0.f);
				lights[id + 1].setBrightness(1.f);
				lights[id + 2].setBrightness(0.f);
				break;
			case BLUE:
				lights[id + 0].setBrightness(0.f);
				lights[id + 1].setBrightness(0.f);
				lights[id + 2].setBrightness(1.f);
				break;
			case YELLOW:
				lights[id + 0].setBrightness(1.f);
				lights[id + 1].setBrightness(1.f);
				lights[id + 2].setBrightness(0.f);
				break;
			case WHITE:
				lights[id + 0].setBrightness(0.5f);
				lights[id + 1].setBrightness(0.5f);
				lights[id + 2].setBrightness(0.5f);
				break;
			default:	
				lights[id + 0].setBrightness(1.f);
				lights[id + 1].setBrightness(1.f);
				lights[id + 2].setBrightness(1.f);
				break;
			}
		}
		else {
			switch (color) {
			case OFF:
				lights[id].setBrightness(0.f);
				break;
			default:	
				lights[id].setBrightness(1.f);
				break;
			}	
		}
	}

	void setWaitForFollowingKey(int param, bool on) {
		if (param == SEL_PARAM) {
			DEBUG("set bWaitForSel %d", on);
			bWaitForSel = on;
		}
		else if (param == COPY_PARAM) {
			DEBUG("set bWaitForCopy %d", on);
			bWaitForCopy = on;
		}
		
		for (int i = 0; i < 12; i++) {
			setLightColor(keyLights[i], on ? YELLOW : OFF);
		}
		if (on) {
			saveINS_PARAM = params[INS_PARAM].getValue();
			saveTRANS_PARAM = params[TRANS_PARAM].getValue();
			params[INS_PARAM].setValue(0.f);
			params[TRANS_PARAM].setValue(0.f);
		}
		else {
			params[INS_PARAM].setValue(saveINS_PARAM);
			params[TRANS_PARAM].setValue(saveTRANS_PARAM);
		}
	}
	
	int getIdxByNoteVoltage(float noteVoltage) {
		int idx = 0;
		if (compareCV(noteVoltage, 0.000000f) == 0) idx = 0;
		if (compareCV(noteVoltage, 0.083333f) == 0) idx = 1;
		if (compareCV(noteVoltage, 0.166667f) == 0) idx = 2;
		if (compareCV(noteVoltage, 0.250000f) == 0) idx = 3;
		if (compareCV(noteVoltage, 0.333333f) == 0) idx = 4;
		if (compareCV(noteVoltage, 0.416667f) == 0) idx = 5;
		if (compareCV(noteVoltage, 0.500000f) == 0) idx = 6;
		if (compareCV(noteVoltage, 0.583333f) == 0) idx = 7;
		if (compareCV(noteVoltage, 0.666667f) == 0) idx = 8;
		if (compareCV(noteVoltage, 0.750000f) == 0) idx = 9;
		if (compareCV(noteVoltage, 0.833333f) == 0) idx = 10;
		if (compareCV(noteVoltage, 0.916667f) == 0) idx = 11;
		return idx;
	}

	void setCmdByCvVoltage(float cvVoltage) {
		// noteVoltage of key without octave
		int octVoltage = (int)floor(cvVoltage); 
		float noteVoltage = cvVoltage - octVoltage; 
		DEBUG("Execute CMD %f", noteVoltage);
		switch (getIdxByNoteVoltage(noteVoltage)) {
		case 0:
			params[PLAY_PARAM].setValue(1.0f);
			break;
		case 1:
			params[PREV_PARAM].setValue(1.0f);
			if (octVoltage <= -1.0) {
				moveSteps = 1;
			}
			else if (octVoltage == 0.0) {
				moveSteps = DISPLAYNOTES_PER_LINE / 2;
			}
			else if (octVoltage >= 1.0) {
				moveSteps = DISPLAYNOTES_PER_LINE;
			}
			setLightColor(PREV_LIGHT, BLUE);
			cmdLight = PREV_LIGHT;
			break;
		case 2:
			params[REST_PARAM].setValue(1.0f);
			setLightColor(REST_LIGHT, BLUE);
			cmdLight = REST_LIGHT;
			break;
		case 3:
			params[NEXT_PARAM].setValue(1.0f);
			if (octVoltage <= -1.0) {
				moveSteps = 1;
			}
			else if (octVoltage == 0.0) {
				moveSteps = DISPLAYNOTES_PER_LINE / 2;
			}
			else if (octVoltage >= 1.0) {
				moveSteps = DISPLAYNOTES_PER_LINE;
			}
			setLightColor(NEXT_LIGHT, BLUE);
			cmdLight = NEXT_LIGHT;
			break;
		case 4:
			params[TIE_PARAM].setValue(1.0f);
			setLightColor(TIE_LIGHT, BLUE);
			cmdLight = TIE_LIGHT;
			break;
		case 5:
			params[DEL_PARAM].setValue(1.0f);
			setLightColor(DEL_LIGHT, RED);
			cmdLight = DEL_LIGHT;
			break;
		case 6:
			params[CLEAR_PARAM].setValue(1.0f);
			break;
		case 7:
			if (params[INS_PARAM].getValue() == 1.f) {
				params[INS_PARAM].setValue(0.f);
				setLightColor(INS_LIGHT, OFF);
			}
			else {
				params[INS_PARAM].setValue(1.f);
				setLightColor(INS_LIGHT, GREEN);
			}
			break;
		case 8:
			params[SQTRA_PARAM].setValue(1.0f);
			break;
		case 9:
			params[SEL_PARAM].setValue(1.0f);
			break;
		case 10:
			params[COPY_PARAM].setValue(1.0f);
			break;
		case 11:
			params[TRANS_PARAM].setValue(1.f);
			break;
		}
	}

	void setTransVoltage() {
		if (bTransMode) {
			if (compareCV(transCVVoltage, newTransCVVoltage) != 0) {
				// update TransCVvoltage on end of seq, if a new value is set
				transCVVoltage = newTransCVVoltage;
				transKeyColor = SCHEME_BLUE;
			}
		}
		else if (inputs[TRANS_INPUT].isConnected()) {
			if (compareCV(transCVVoltage, lastTransInputCVVoltage) != 0) {
				transCVVoltage = lastTransInputCVVoltage;
				transKeyColor = SCHEME_GREEN;
				fillNoteFromVoltage(transKey, transCVVoltage);
			}
		}
		else if (pSeqTRANS->len > 0) {
			if (pSeqTRANS->isAfterLastStep()) {
				pSeqTRANS->reset();
			}
			if (pSeqTRANS->getGatemode() == GATEMODE_NORMAL) {
				transCVVoltage = pSeqTRANS->getCV();
				transKeyColor = SCHEME_PURPLE;
				fillNoteFromVoltage(transKey, transCVVoltage);
				char keyNameDbg[8];
				fillNoteFromVoltage(keyNameDbg, transCVVoltage);
				DEBUG("%d trans CV=%f Note=%s", 
					pSeqTRANS->pos, transCVVoltage, keyNameDbg);
			}
			pSeqTRANS->next();
		}
		else {
			transCVVoltage = 0.f; // C4
			fillNoteFromVoltage(transKey, transCVVoltage);
			transKeyColor = SCHEME_WHITE;
		}
	}
	
	void copyRecSequenceToPlay() {
		// Copy new seqREC, if available on end of seq
		DEBUG("copy rec seq %d to play", seqRECToPLAYIdx);
		if (seqRECToPLAYIdx == 11) {
			pSeqTRANS->copy(&seqREC[seqRECToPLAYIdx]);
			seqTRANSIdx = seqRECToPLAYIdx;
		}
		else {
			pSeqPLAY->copy(&seqREC[seqRECToPLAYIdx]);
			seqPLAYIdx = seqRECToPLAYIdx;
		}
		seqRECToPLAYIdx =- 1;
		setLightColor(PLAY_LIGHT, OFF);
		// pSeqPLAY->dump("AfterCopy");
	}
	
	void selectSequence(int idx) {
		DEBUG("select rec seq %d ", idx);
		seqRECIdx = idx;
		pSeqREC = &(seqREC[seqRECIdx]);
		setWaitForFollowingKey(SEL_PARAM, false);
		displayNeedsUpdate = true;
		seqNoteDiplayColor = seqRECIdx == 11 ? SCHEME_PURPLE : SCHEME_YELLOW;
		if ((bAutoPlayOnSeqSelected || bAutoPlayAlways) && seqRECIdx != 11) {
			seqRECToPLAYIdx = seqRECIdx;
		}
	}

	void copyRecSequenceToOther(int otherIdx) {
		DEBUG("select rec seq %d to other rec seq %d seqRECIdx", otherIdx, seqRECIdx);
		seqREC[otherIdx].copy(pSeqREC);
		seqREC[otherIdx].pos = pSeqREC->pos;
		setWaitForFollowingKey(COPY_PARAM, false);
		if (bSetSeqOnCopy) {
			selectSequence(otherIdx);
		}
	}
	
	void process(const ProcessArgs& args) override {
		if (args.frame % FRAME_DIVIDER != 0)
			return;

		if (this->thread_flag.test_and_set())
			return;

		if (params[RECORD_PARAM].getValue() > 0) {
			//
			// handle CLK_INPUT; play notes
			bool bClock = clock.process(inputs[CLK_INPUT].getVoltage());
			if (bClock && params[RUN_PARAM].getValue() > 0.f) {

				if (pSeqPLAY->isAfterLastStep()) {
					gatelenEOS = 200;
					if (bCopyOnEOS && seqRECToPLAYIdx >= 0) {
						copyRecSequenceToPlay();
					}
				
					if (bTransOnEOS) {
						setTransVoltage();
					}
					
					// End of Seq - start at begin
					pSeqPLAY->reset();
				}
				
				if (!pSeqPLAY->isAfterLastStep()) {
					// seq contains a new step

					if (!bCopyOnEOS && seqRECToPLAYIdx >= 0) {
						copyRecSequenceToPlay();
					}

					if (!bTransOnEOS) {
						setTransVoltage();
					}
					
					// set CV, gatelen and velocity
					float cvVoltage = pSeqPLAY->getCV() + transCVVoltage;
					int gatemode = pSeqPLAY->getGatemode();
					int gatemode_next = pSeqPLAY->getGatemode(1);
					float velVoltage = pSeqPLAY->getVelocity();
					if (gatemode == GATEMODE_NORMAL || gatemode == GATEMODE_TIED) {
						outputs[CV_OUTPUT].setVoltage(cvVoltage);
						outputs[VEL_OUTPUT].setVoltage(velVoltage);
						if (gatemode_next == GATEMODE_TIED) {
							gatelen = 5000; // means until next step (or longer)
						}
						else {
							gatelen = params[GATELEN_PARAM].getValue();
						}
					}
					else if (gatemode == GATEMODE_REST) {
						outputs[CV_OUTPUT].setVoltage(0.f);
						outputs[VEL_OUTPUT].setVoltage(0.f);
						gatelen = 0; // no gate on a rest
					}
					
					char keyNameDbg[8];
					fillNoteFromVoltage(keyNameDbg, cvVoltage);
					DEBUG("%d play CV=%f Vel=%f Gate=%d Gatemode=%d Note=%s", 
						pSeqPLAY->pos, cvVoltage, velVoltage, gatelen, gatemode, keyNameDbg);
						
					pSeqPLAY->next();
				}
				else {
					// seq is empty (contains only GATEMODE_EOS)
				}
			}
			//
			// set GATE_OUTPUT; play notes
			if (gatelen > 0) {
				outputs[GATE_OUTPUT].setVoltage(10.f);
				gatelen--;
			}
			else {
				outputs[GATE_OUTPUT].setVoltage(0.f);
			}
		}
		else {
			// send inputs to outputs
			outputs[CV_OUTPUT].setVoltage(inputs[CV_INPUT].getVoltage());
			outputs[GATE_OUTPUT].setVoltage(inputs[GATE_INPUT].getVoltage());
			if (inputs[VEL_INPUT].isConnected()) {
				outputs[VEL_OUTPUT].setVoltage(inputs[VEL_INPUT].getVoltage());
			}
			else {
				outputs[VEL_OUTPUT].setVoltage(10.f);
			}
		}
		
		// set END_OUTPUT
		if (gatelenEOS > 0) {
			outputs[END_OUTPUT].setVoltage(10.f);
			gatelenEOS--;
		}
		else {
			outputs[END_OUTPUT].setVoltage(0.f);
		}

		//
		// handle _INPUT and _PARAM
		if (inputs[RST_INPUT].getVoltage() > 0.0f && params[RST_PARAM].getValue() == 0.f) {
			params[RST_PARAM].setValue(1.f);
		}
		
		if (params[RST_PARAM].getValue() > 0.f) {
			DEBUG("Reset");
			pSeqPLAY->reset();
			params[RST_PARAM].setValue(0.f);
			gatelen = 0;
			gatelenEOS = 0;
		}

		bool bRun = run.process(inputs[RUN_INPUT].getVoltage());
		if (bRun) {
			if (params[RUN_PARAM].getValue() > 0.f) {
				params[RUN_PARAM].setValue(0.f);
				gatelen = 0;
				gatelenEOS = 0;
				DEBUG("Stop run");
			}
			else {
				params[RUN_PARAM].setValue(1.f);
				DEBUG("Start run");
			}
		}

		float copyInputVoltage = inputs[PLAY_INPUT].getVoltage();
		if (compareCV(copyInputVoltage, lastCopyVoltage) != 0) {
			lastCopyVoltage = copyInputVoltage;
			int newSeqPLAYIdx = -1;
			if (bPlayUse10) {
				newSeqPLAYIdx = round((copyInputVoltage/10.0f) * NUM_SEQ);
			}
			else {
				float noteVoltage = copyInputVoltage - floor(copyInputVoltage); 
				newSeqPLAYIdx = getIdxByNoteVoltage(noteVoltage);
			}
			if (newSeqPLAYIdx != seqPLAYIdx && newSeqPLAYIdx >= 0 && newSeqPLAYIdx <= NUM_SEQ) {
				if (bSelectOnPlayInput) {
					selectSequence(newSeqPLAYIdx);
				}
				else {
					seqRECToPLAYIdx = newSeqPLAYIdx;
				}
			}
		}

		float transInputVoltage = inputs[TRANS_INPUT].getVoltage();
		if (compareCV(transInputVoltage, lastTransInputCVVoltage) != 0) {
			lastTransInputCVVoltage = transInputVoltage;
			transKeyColor = SCHEME_YELLOW;
			fillNoteFromVoltage(transKey, transInputVoltage);

		}

		if (params[RECORD_PARAM].getValue() > 0) {

			//
			// handle GATE_INPUT; record notes and commands
			int iGateEvent = gatein.processEvent(inputs[GATE_INPUT].getVoltage());
			if (iGateEvent == dsp::TSchmittTrigger<float>::Event::TRIGGERED) {
				// Key pressed
				gateStartCvVoltage = inputs[CV_INPUT].getVoltage();
				DEBUG("Key pressed CV=%f", gateStartCvVoltage);
				int idx = getIdxByNoteVoltage(gateStartCvVoltage - floor(gateStartCvVoltage));
				setLightColor(keyLights[idx], WHITE);
				bRecordNote = true;
				if (bKeyboardControl) {
					bWaitForCmd = true;
				}
				setLightColor(KEYPRESS_LIGHT, ON);
			}
			else if (gatein.isHigh()) { 
				// Key IS pressed
				float cvVoltage = inputs[CV_INPUT].getVoltage();
				if (bWaitForCmd && compareCV(gateStartCvVoltage, cvVoltage) != 0) {
					// waiting for command (bWaitForCmd)
					// and a second key was pressed (CV Voltage has changed)
					// -> set _PARAM voltage to execute command
					
					if (bCmdFromLastKey) {
						// set cmd from first key
						setCmdByCvVoltage(cvVoltage);
					}
					else {
						// set cmd from last key
						setCmdByCvVoltage(gateStartCvVoltage);
					}
					bWaitForCmd = false;
					bRecordNote = false; // after a command no note should be recorded
				}
				else if (!bWaitForCmd && compareCV(gateStartCvVoltage, cvVoltage) == 0) {
					// Command was set/executed in last process-loop (!bWaitForCmd)
					// and second key was released // (CV voltage is the same as at first keypress)
					// -> wait for next command
					if (bKeyboardControl) {
						bWaitForCmd = true;
					}
					if (cmdLight != -1) {
						setLightColor(cmdLight, OFF);
						cmdLight = -1;
					}
					if (! (bWaitForSel || bWaitForCopy)) {
						int idx = getIdxByNoteVoltage(gateStartCvVoltage - floor(gateStartCvVoltage));
						setLightColor(keyLights[idx], WHITE);
					}
				}
			}

			//
			// record note on key release
			if (iGateEvent == dsp::TSchmittTrigger<float>::Event::UNTRIGGERED) {
				setLightColor(KEYPRESS_LIGHT, OFF);

				if (bRecordNote) {
					// Key was pressed an no command was executed (bRecordNote)
					float cvVoltage = inputs[CV_INPUT].getVoltage();
					float velVoltage = 10.0f;
					if (inputs[VEL_INPUT].isConnected()) {
						velVoltage = inputs[VEL_INPUT].getVoltage();
					}
					DEBUG("Key released CV=%f", cvVoltage);
					int idx = getIdxByNoteVoltage(gateStartCvVoltage - floor(gateStartCvVoltage));
					setLightColor(keyLights[idx], OFF);
					
					if ((bWaitForSel || bWaitForCopy) && bKeyboardControl) {
						// use note for seq index
						float noteVoltage = cvVoltage - floor(cvVoltage); 
						int idx = getIdxByNoteVoltage(noteVoltage);
						if (bWaitForSel) { 
							selectSequence(idx);
						}
						else if (bWaitForCopy) {
							copyRecSequenceToOther(idx);
						}
					}
					else if (bTransMode) {
						DEBUG("Transpose by note");
						// use note for translation
						newTransCVVoltage = cvVoltage;
						fillNoteFromVoltage(transKey, newTransCVVoltage);
						transKeyColor = SCHEME_YELLOW;
					}
					else {
						DEBUG("Record Note");
	#ifdef SMART_TIE					
						int64_t diffFrames = args.frame - lastFrame;
						int diffTime = 	(int)(diffFrames*APP->engine->getSampleTime()*1000);
						float lastCVVoltage = pSeqREC->getCV(-1);
						int lastGatemode = pSeqREC->getGatemode(-1);
						float lastVelocity = pSeqREC->getVelocity(-1);
						DEBUG("difftime=%d ms", diffTime);
						if (diffTime > params[TIECLICK_PARAM].getValue() && diffTime < 5000 && !bFirstStep && 
							compareCV(lastCVVoltage, cvVoltage) == 0 &&
							lastGatemode != GATEMODE_REST
							) {
							// 	Slow press of same key -> add a tied note
							DEBUG("%d record TIED 2", pSeqREC->pos);
							if (params[INS_PARAM].getValue() == 1.f) {
								pSeqREC->insertStep(lastCVVoltage, GATEMODE_TIED, lastVelocity);
							}
							else {
								pSeqREC->setStep(lastCVVoltage, GATEMODE_TIED, lastVelocity);
							}
							if ((bAutoPlayOnNewNote || bAutoPlayAlways) && seqRECIdx != 11) {
								seqRECToPLAYIdx = seqRECIdx;
							}
						}
						else {
	#endif						
						DEBUG("%d record CV=%f Time=%ld", pSeqREC->pos, cvVoltage, args.frame - lastFrame);
						if (params[INS_PARAM].getValue() == 1.f) {
							pSeqREC->insertStep(cvVoltage, GATEMODE_NORMAL, velVoltage);
						}
						else {
							pSeqREC->setStep(cvVoltage, GATEMODE_NORMAL, velVoltage);
						}
						if ((bAutoPlayOnNewNote || bAutoPlayAlways) && seqRECIdx != 11) {
							seqRECToPLAYIdx = seqRECIdx;
						}
	#ifdef SMART_TIE					
						}
	#endif
						if (pSeqREC->next()) {
							displayNeedsUpdate = true;
						}
						lastFrame = args.frame;
					}
					bRecordNote = false;
				}
				else {
					if (! (bWaitForSel || bWaitForCopy)) {
						float cvVoltage = inputs[CV_INPUT].getVoltage();
						int idx = getIdxByNoteVoltage(cvVoltage - floor(cvVoltage));
						setLightColor(keyLights[idx], OFF);
					}
					bWaitForCmd = false;
				}
			}
		}
		
		if (inputs[CV2_INPUT].isConnected() && inputs[GATE2_INPUT].isConnected()) {
			int iGateEvent2 = gatein2.processEvent(inputs[GATE2_INPUT].getVoltage());
			if (iGateEvent2 == dsp::TSchmittTrigger<float>::Event::TRIGGERED) {
				float cvVoltage = inputs[CV2_INPUT].getVoltage();
				setLightColor(KEYPRESS_LIGHT, ON);

				if (bWaitForSel || bWaitForCopy) {
					// use note for seq index
					float noteVoltage = cvVoltage - floor(cvVoltage); 
					int idx = getIdxByNoteVoltage(noteVoltage);
					if (bWaitForSel) { 
						selectSequence(idx);
					}
					else if (bWaitForCopy) {
						copyRecSequenceToOther(idx);
					}
				}
				else {
					setCmdByCvVoltage(inputs[CV2_INPUT].getVoltage());
				}
			}
			if (iGateEvent2 == dsp::TSchmittTrigger<float>::Event::UNTRIGGERED) {
				setLightColor(KEYPRESS_LIGHT, OFF);
				if (cmdLight != -1) {
					setLightColor(cmdLight, OFF);
					cmdLight = -1;
				}
			}
		}

		//
		// handle command (_PARAM switches)
		if (bWaitForSel || bWaitForCopy) {
			// command defines sequence
			int idx = -1;	
			int paramToSel[] = {PLAY_PARAM, PREV_PARAM, REST_PARAM, NEXT_PARAM, TIE_PARAM, DEL_PARAM, 
				CLEAR_PARAM, INS_PARAM, SQTRA_PARAM, SEL_PARAM, COPY_PARAM, TRANS_PARAM};
			for (int i = 0; i < (int)(sizeof(paramToSel)/sizeof(int)); i++) {
				if (params[paramToSel[i]].getValue() > 0.f) {
					idx = i;
				}
			}
			if (idx >= 0) {
				if (bWaitForSel) {
					selectSequence(idx);
				}
				else if (bWaitForCopy) {
					copyRecSequenceToOther(idx);
				}
			}
		}
		else {
			// handle command (_PARAM switches)
			if (params[PLAY_PARAM].getValue() > 0.f) {
				seqRECToPLAYIdx = seqRECIdx;
				params[PLAY_PARAM].setValue(0.f);
			}
			if (params[PREV_PARAM].getValue() > 0.f) {
				DEBUG("handle PREV");
				if (pSeqREC->prev(moveSteps)) {
					displayNeedsUpdate = true;
				}
				params[PREV_PARAM].setValue(0.f);
			}
			
			if (params[NEXT_PARAM].getValue() > 0.f) {
				DEBUG("handle NEXT");
				if (pSeqREC->next(moveSteps)) {
					displayNeedsUpdate = true;
				}
				params[NEXT_PARAM].setValue(0.f);
			}
			
			if (params[REST_PARAM].getValue() > 0.f) {
				DEBUG("%d record REST", pSeqREC->pos);
				if (params[INS_PARAM].getValue() == 1.f) {
					pSeqREC->insertStep(-10.f, GATEMODE_REST, 0.f);
				}
				else {
					pSeqREC->setStep(-10.f, GATEMODE_REST, 0.f);
				}
				if (pSeqREC->next()) {
					displayNeedsUpdate = true;
				}
				params[REST_PARAM].setValue(0.f);
				if ((bAutoPlayOnNewRest || bAutoPlayAlways) && seqRECIdx != 11) {
					seqRECToPLAYIdx = seqRECIdx;
				}
			}

			if (params[TIE_PARAM].getValue() > 0.f) {
				bool bFirstStep = pSeqREC->isFirstStep();
				float lastCVVoltage = pSeqREC->getCV(-1);
				int lastGatemode = pSeqREC->getGatemode(-1);
				float lastVelocity = pSeqREC->getVelocity(-1);
				if (!bFirstStep && lastGatemode != GATEMODE_REST) {
					DEBUG("%d record TIED", pSeqREC->pos);
					if (params[INS_PARAM].getValue() == 1.f) {
						pSeqREC->insertStep(lastCVVoltage, GATEMODE_TIED, lastVelocity);
					}
					else {
						pSeqREC->setStep(lastCVVoltage, GATEMODE_TIED, lastVelocity);
					}
					if (pSeqREC->next()) {
						displayNeedsUpdate = true;
					}
				}
				params[TIE_PARAM].setValue(0.f);
				if ((bAutoPlayOnNewTie || bAutoPlayAlways) && seqRECIdx != 11) {
					seqRECToPLAYIdx = seqRECIdx;
				}
			}
			
			if (params[CLEAR_PARAM].getValue() > 0.f && !pSeqREC->isEmpty()) {
				DEBUG("Clear record");
				pSeqREC->clear();
				displayNeedsUpdate = true;
				params[CLEAR_PARAM].setValue(0.f);
			}
			
			if (params[DEL_PARAM].getValue() > 0.f) {
				DEBUG("Delete Note");
				if (pSeqREC->deleteStep()) {
					displayNeedsUpdate = true;
				}
				params[DEL_PARAM].setValue(0.f);
				if ((bAutoPlayOnDelete || bAutoPlayAlways) && seqRECIdx != 11) {
					seqRECToPLAYIdx = seqRECIdx;
				}
			}
			
			if (params[SEL_PARAM].getValue() > 0.f) {
				setWaitForFollowingKey(SEL_PARAM, true);
				params[SEL_PARAM].setValue(0.f);
			}
			
			if (params[COPY_PARAM].getValue() > 0.f) {
				setWaitForFollowingKey(COPY_PARAM, true);
				params[COPY_PARAM].setValue(0.f);
			}
			
			if (params[TRANS_PARAM].getValue() > 0.f) {
				bTransMode = ! bTransMode;
				params[TRANS_PARAM].setValue(0.f);
			}

		}
		
		if (lightDivider.process() && ! (bWaitForSel || bWaitForCopy || bRecordNote)) {
			setLightColor(RUN_LIGHT, params[RUN_PARAM].getValue() > 0 ? ON : OFF);
			setLightColor(CLEAR_LIGHT, pSeqREC->len == 0 ? GREEN : OFF);
			setLightColor(TRANS_LIGHT, bTransMode ? BLUE : OFF);
			setLightColor(INS_LIGHT, params[INS_PARAM].getValue() > 0 ? GREEN : OFF);
			setLightColor(PLAY_LIGHT, seqRECToPLAYIdx >= 0 ? YELLOW : OFF);
			setLightColor(RECORD_LIGHT, params[RECORD_PARAM].getValue() > 0 ? ON : OFF);
		}
		
		if (displayNeedsUpdate) {
			updateDisplayNotes();
			displayNeedsUpdate = false;
		}
		thread_flag.clear();
	}
	
	json_t* dataToJson(void) override {
		/*
		"sequences": [
			{ "type": "play", "idx": 0, "len": 10, "pos": 3, "data": "seqstr" }
		]
		*/	
		json_t* j_root = json_object();
		json_t* j_sequences = json_array();
		
		// seqPLAY
		json_t* j_seq = json_object();
		json_object_set_new(j_seq, "type", json_string("PLAY"));
		json_object_set_new(j_seq, "idx", json_integer(0));
		json_object_set_new(j_seq, "len", json_integer(seqPLAY.len));
		json_object_set_new(j_seq, "pos", json_integer(seqPLAY.pos));
		seqPLAY.createString(seqTextBuff);
		json_object_set_new(j_seq, "data", json_string(seqTextBuff));
		json_array_append_new(j_sequences, j_seq);
		
		// seqTRANS
		j_seq = json_object();
		json_object_set_new(j_seq, "type", json_string("TRANS"));
		json_object_set_new(j_seq, "idx", json_integer(0));
		json_object_set_new(j_seq, "len", json_integer(seqTRANS.len));
		json_object_set_new(j_seq, "pos", json_integer(seqTRANS.pos));
		seqTRANS.createString(seqTextBuff);
		json_object_set_new(j_seq, "data", json_string(seqTextBuff));
		json_array_append_new(j_sequences, j_seq);

		// seqREC
		for (int i = 0; i < NUM_SEQ; i++) {
			if (seqREC[i].len == 0) {
				continue;
			}
			j_seq = json_object();
			json_object_set_new(j_seq, "type", json_string("REC"));
			json_object_set_new(j_seq, "idx", json_integer(i));
			json_object_set_new(j_seq, "len", json_integer(seqREC[i].len));
			json_object_set_new(j_seq, "pos", json_integer(seqREC[i].pos));
			seqREC[i].createString(seqTextBuff);
			json_object_set_new(j_seq, "data", json_string(seqTextBuff));
			json_array_append_new(j_sequences, j_seq);
		}

		json_object_set_new(j_root, "seqPLAYIdx", json_integer(seqPLAYIdx));
		json_object_set_new(j_root, "seqRECIdx", json_integer(seqRECIdx));
		json_object_set_new(j_root, "seqTRANSIdx", json_integer(seqPLAYIdx));
		
		json_object_set_new(j_root, "sequences", j_sequences);

		json_object_set_new(j_root, "CmdFromLastKey", json_boolean(bCmdFromLastKey));
		json_object_set_new(j_root, "AutoPlayOnNewNote", json_boolean(bAutoPlayOnNewNote));
		json_object_set_new(j_root, "AutoPlayOnNewRest", json_boolean(bAutoPlayOnNewRest));
		json_object_set_new(j_root, "AutoPlayOnNewTie", json_boolean(bAutoPlayOnNewTie));
		json_object_set_new(j_root, "AutoPlayOnDelete", json_boolean(bAutoPlayOnDelete));
		json_object_set_new(j_root, "AutoPlayOnSeqSelected", json_boolean(bAutoPlayOnSeqSelected));
		json_object_set_new(j_root, "AutoPlayAlways", json_boolean(bAutoPlayAlways));
		json_object_set_new(j_root, "CopyUse10", json_boolean(bPlayUse10));
		json_object_set_new(j_root, "CopyOnEOS", json_boolean(bCopyOnEOS));
		json_object_set_new(j_root, "TransOnEOS", json_boolean(bTransOnEOS));
		json_object_set_new(j_root, "SetSeqOnCopy", json_boolean(bSetSeqOnCopy));
		json_object_set_new(j_root, "KeyboardControl", json_boolean(bKeyboardControl));
		json_object_set_new(j_root, "SelectOnPlayInput", json_boolean(bSelectOnPlayInput));

		return j_root;
	}
	
	void dataFromJson(json_t *j_root) override {
		DEBUG("dataFromJson");
		json_t* j_sequences = json_object_get(j_root, "sequences");
		if (j_sequences && json_typeof(j_sequences) == JSON_ARRAY) {
			int j_arraysize = json_array_size(j_sequences);
			DEBUG("dataFromJson j_arraysize=%d", j_arraysize);
			for (int i = 0; i < j_arraysize; i++) {
				json_t* j_seq = json_array_get(j_sequences, i);
				char* type = (char*)json_string_value(json_object_get(j_seq, "type"));
				int idx = (int)json_integer_value(json_object_get(j_seq, "idx"));
				int len = (int)json_integer_value(json_object_get(j_seq, "len"));
				int pos = (int)json_integer_value(json_object_get(j_seq, "pos"));
				char* data = (char*)json_string_value(json_object_get(j_seq, "data"));
				DEBUG("dataFromJson type=%s idx=%d len=%d pos=%d data=%s", type, idx, len, pos, data);
				if (strcmp(type, "PLAY") == 0) {
					seqPLAY.len = len;
					seqPLAY.pos = pos;
					seqPLAY.createFromString(data);
				}
				else if (strcmp(type, "TRANS") == 0) {
					seqTRANS.len = len;
					seqTRANS.pos = pos;
					seqTRANS.createFromString(data);
				}
				else if (strcmp(type, "REC") == 0) {
					seqREC[idx].len = len;
					seqREC[idx].pos = pos;
					seqREC[idx].createFromString(data);
				}
			}
		}
		seqPLAYIdx = (int)json_integer_value(json_object_get(j_root, "seqPLAYIdx"));
		pSeqPLAY = &(seqPLAY);
		seqTRANSIdx = (int)json_integer_value(json_object_get(j_root, "seqTRANSIdx"));
		pSeqTRANS = &(seqTRANS);
		seqRECIdx = (int)json_integer_value(json_object_get(j_root, "seqRECIdx"));
		pSeqREC = &(seqREC[seqRECIdx]);
		seqNoteDiplayColor = seqRECIdx == 11 ? SCHEME_PURPLE : SCHEME_YELLOW;
		
		bCmdFromLastKey = json_boolean_value(json_object_get(j_root, "CmdFromLastKey"));
		bAutoPlayOnNewNote = json_boolean_value(json_object_get(j_root, "AutoPlayOnNewNote"));
		bAutoPlayOnNewRest = json_boolean_value(json_object_get(j_root, "AutoPlayOnNewRest"));
		bAutoPlayOnNewTie = json_boolean_value(json_object_get(j_root, "AutoPlayOnNewTie"));
		bAutoPlayOnDelete = json_boolean_value(json_object_get(j_root, "AutoPlayOnDelete"));
		bAutoPlayOnSeqSelected = json_boolean_value(json_object_get(j_root, "AutoPlayOnSeqSelected"));
		bAutoPlayAlways = json_boolean_value(json_object_get(j_root, "AutoPlayAlways"));
		bPlayUse10 = json_boolean_value(json_object_get(j_root, "CopyUse10"));
		bCopyOnEOS = json_boolean_value(json_object_get(j_root, "CopyOnEOS"));
		bTransOnEOS = json_boolean_value(json_object_get(j_root, "TransOnEOS"));
		bSetSeqOnCopy = json_boolean_value(json_object_get(j_root, "SetSeqOnCopy"));
		bKeyboardControl = json_boolean_value(json_object_get(j_root, "KeyboardControl"));
		bSelectOnPlayInput = json_boolean_value(json_object_get(j_root, "SelectOnPlayInput"));

		displayNeedsUpdate = true;
	}

};

struct Display : Widget {
	KeySeq* module;
	std::string fontPath;
	std::string bgText;
	std::string text;
	float fontSize;
	NVGcolor bgColor = nvgRGB(0x46,0x46, 0x46);
	NVGcolor fgColor = SCHEME_YELLOW;
	Vec textPos;

	void prepareFont(const DrawArgs& args) {
		// Get font
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
		if (!font)
			return;
		nvgFontFaceId(args.vg, font->handle);
		nvgFontSize(args.vg, fontSize);
		nvgTextLetterSpacing(args.vg, 0.0);
		nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
	}

	void draw(const DrawArgs& args) override {
		// Background
		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2);
		nvgFillColor(args.vg, nvgRGB(0x19, 0x19, 0x19));
		nvgFill(args.vg);

		prepareFont(args);

		// Background text
		nvgFillColor(args.vg, bgColor);
		nvgText(args.vg, textPos.x, textPos.y, bgText.c_str(), NULL);
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer == 1) {
			prepareFont(args);

			// Foreground text
			nvgFillColor(args.vg, fgColor);
			nvgText(args.vg, textPos.x, textPos.y, text.c_str(), NULL);
		}
		Widget::drawLayer(args, layer);
	}

};

struct StepDisplay : Display {
	StepDisplay() {
		fontPath = asset::system("res/fonts/DSEG7ClassicMini-BoldItalic.ttf");
		textPos = Vec(42, 22);
		bgText = "888";
		text = "001";
		fontSize = 16;
	}
};

struct RecStepDisplay : StepDisplay {
	void step() override {
		if (module) {
			text = string::f("%d", module->pSeqREC->pos + 1);
		}
	}

	RecStepDisplay() : StepDisplay() {
		fgColor = SCHEME_YELLOW;
	}
};

struct PlayStepDisplay : StepDisplay {
	void step() override {
		if (module) {
			text = string::f("%d", module->pSeqPLAY->pos);
		}
	}

	PlayStepDisplay() : StepDisplay() {
		fgColor = SCHEME_GREEN;
	}
};

struct KeyDisplay : Display {
	KeyDisplay() {
		fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
		textPos = Vec(28, 15);
		bgText = "___";
		text = "A3#";
		fontSize = 15;
	}
};

struct TransKeyDisplay : KeyDisplay {
	void step() override {
		if (module) {
			text = string::f("%s", module->transKey);
			fgColor = module->transKeyColor;
		}
	}
};

struct RecSeqDisplay : KeyDisplay {
	void step() override {
		if (module) {
			text = string::f("s%02d", module->seqRECIdx);
			fgColor = SCHEME_YELLOW;
		}
	}
};

struct RecLenDisplay : KeyDisplay {
	void step() override {
		if (module) {
			text = string::f("%d", module->pSeqREC->len);
			fgColor = SCHEME_YELLOW;
		}
	}
};

struct PlaySeqDisplay : KeyDisplay {
	void step() override {
		if (module) {
			text = string::f("s%02d", module->seqPLAYIdx);
			fgColor = SCHEME_GREEN;
		}
	}
};

struct PlayLenDisplay : KeyDisplay {
	void step() override {
		if (module) {
			text = string::f("%d", module->pSeqPLAY->len);
			fgColor = SCHEME_GREEN;
		}
	}
};

struct SeqNoteDisplayPlay : Display {
	int dispPos;
	
	SeqNoteDisplayPlay() {
		fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
		fontSize = 18;
		textPos = Vec(28, 16);
		bgText = "   ";
		text =   "C4#";
		dispPos = 0;
	}

	void draw(const DrawArgs& args) override {
		if (module) {
			if (text.length() == 1) {
				textPos = Vec(21, 16);
			}
			else if (text.length() == 2) {
				textPos = Vec(24, 16);
			}
			else {
				textPos = Vec(28, 16);
			}
			Display::draw(args);
			// Background
			if (dispPos == DISPLAYNOTE_ACT) {
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2);
				nvgFillColor(args.vg, nvgRGB(0x8f, 0x8f, 0x8f));
				nvgFill(args.vg);
			}
		}
	}
	
	void step() override {
		if (module) {
			text = string::f("%s", module->displayNotes[dispPos]);
			if (dispPos == DISPLAYNOTE_ACT) {
				fgColor = SCHEME_ORANGE;
			}
			else {
				fgColor = module->seqNoteDiplayColor;
			}
		}
	}
};


struct KeySeqWidget : ModuleWidget {
	KeySeqWidget(KeySeq* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/KeySeq.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

#ifdef SMART_TIE
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(44.027, 26.416)), module, KeySeq::TIECLICK_PARAM));
#endif
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(94.827, 26.814)), module, KeySeq::GATELEN_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.467, 16.256)), module, KeySeq::CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(69.088, 16.256)), module, KeySeq::RUN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.467, 26.416)), module, KeySeq::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(69.088, 26.416)), module, KeySeq::CLK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.467, 36.81)), module, KeySeq::VEL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(69.088, 36.576)), module, KeySeq::RST_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(69.088, 46.736)), module, KeySeq::PLAY_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(69.088, 56.896)), module, KeySeq::TRANS_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.467, 103.383)), module, KeySeq::CV2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.467, 115.994)), module, KeySeq::GATE2_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(113.453, 16.256)), module, KeySeq::CV_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(113.453, 26.416)), module, KeySeq::GATE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(113.453, 36.576)), module, KeySeq::VEL_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(113.453, 46.736)), module, KeySeq::END_OUTPUT));

		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(mm2px(Vec(86.021, 16.02)), module, KeySeq::RUN_PARAM, KeySeq::RUN_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(86.021, 36.567)), module, KeySeq::RST_PARAM, KeySeq::RST_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(27.432, 102.277)), module, KeySeq::PREV_PARAM, KeySeq::PREV_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(40.979, 102.277)), module, KeySeq::NEXT_PARAM, KeySeq::NEXT_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(68.072, 102.277)), module, KeySeq::CLEAR_PARAM, KeySeq::CLEAR_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(81.619, 102.277)), module, KeySeq::SQTRA_PARAM, KeySeq::SQTRA_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(95.165, 102.277)), module, KeySeq::COPY_PARAM, KeySeq::COPY_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.997, 111.083)), module, KeySeq::PLAY_PARAM, KeySeq::PLAY_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(34.544, 111.083)), module, KeySeq::REST_PARAM, KeySeq::REST_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(47.413, 111.083)), module, KeySeq::TIE_PARAM, KeySeq::TIE_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(61.637, 111.083)), module, KeySeq::DEL_PARAM, KeySeq::DEL_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(75.184, 111.083)), module, KeySeq::INS_PARAM, KeySeq::INS_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(88.731, 111.083)), module, KeySeq::SEL_PARAM, KeySeq::SEL_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(102.277, 111.083)), module, KeySeq::TRANS_PARAM, KeySeq::TRANS_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(mm2px(Vec(32.512, 27.093)), module, KeySeq::RECORD_PARAM, KeySeq::RECORD_LIGHT));

		addChild(createLightCentered<LargeLight<YellowLight>>(mm2px(Vec(114.043, 107.058)), module, KeySeq::KEYPRESS_LIGHT));

		TransKeyDisplay* transKeyDisplay = createWidget<TransKeyDisplay>(Vec(258.0, 158.0));
		transKeyDisplay->box.size = Vec(35.0, 20.0);
		transKeyDisplay->module = module;
		addChild(transKeyDisplay);
		
		PlayStepDisplay* playStepDisplay = createWidget<PlayStepDisplay>(Vec(300.0, 176.0));
		playStepDisplay->box.size = Vec(44.0, 26.0);
		playStepDisplay->module = module;
		addChild(playStepDisplay);

		PlayLenDisplay* playLenDisplay = createWidget<PlayLenDisplay>(Vec(258.0, 182.0));
		playLenDisplay->box.size = Vec(35.0, 20.0);
		playLenDisplay->module = module;
		addChild(playLenDisplay);

		PlaySeqDisplay* playSeqDisplay = createWidget<PlaySeqDisplay>(Vec(217.0, 182.0));
		playSeqDisplay->box.size = Vec(35.0, 20.0);
		playSeqDisplay->module = module;
		addChild(playSeqDisplay);

		RecStepDisplay* recStepDisplay = createWidget<RecStepDisplay>(Vec(15.0, 176.0));
		recStepDisplay->box.size = Vec(44.0, 26.0);
		recStepDisplay->module = module;
		addChild(recStepDisplay);

		RecLenDisplay* recLenDisplay = createWidget<RecLenDisplay>(Vec(66.0, 182.0));
		recLenDisplay->box.size = Vec(35.0, 20.0);
		recLenDisplay->module = module;
		addChild(recLenDisplay);
		
		RecSeqDisplay* recSeqDisplay = createWidget<RecSeqDisplay>(Vec(109.0, 182.0));
		recSeqDisplay->box.size = Vec(35.0, 20.0);
		recSeqDisplay->module = module;
		addChild(recSeqDisplay);

		float x = 15.0;
		float y = 188.0;
		for (int i = 0; i < DISPLAYNOTES; i++) {
			if ((i % DISPLAYNOTES_PER_LINE) == 0) {
				x = 15.0;
				y = y + 20.0;
			}
			SeqNoteDisplayPlay* seqNoteDisplay = createWidget<SeqNoteDisplayPlay>(Vec(x, y));
			seqNoteDisplay->dispPos = i;
			seqNoteDisplay->box.size = Vec(30.0, 20.0);
			seqNoteDisplay->module = module;
			addChild(seqNoteDisplay);
			x = x + 30.0;
		}
	}
	
	void appendContextMenu(Menu* menu) override {
		KeySeq* module = getModule<KeySeq>();

		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Settings"));

		menu->addChild(createBoolPtrMenuItem("Cmd from keyboard", "", &module->bKeyboardControl));
		menu->addChild(createBoolPtrMenuItem("Cmd from last key", "", &module->bCmdFromLastKey));
		menu->addChild(createSubmenuItem("AutoPlay", "",
			[=](Menu* menu) {
				menu->addChild(createBoolPtrMenuItem("Always", "", &module->bAutoPlayAlways));
				menu->addChild(createBoolPtrMenuItem("When new note", "", &module->bAutoPlayOnNewNote));
				menu->addChild(createBoolPtrMenuItem("When new rest", "", &module->bAutoPlayOnNewRest));
				menu->addChild(createBoolPtrMenuItem("When new tie", "", &module->bAutoPlayOnNewTie));
				menu->addChild(createBoolPtrMenuItem("When delete", "", &module->bAutoPlayOnDelete));
				menu->addChild(createBoolPtrMenuItem("When select sequence", "", &module->bAutoPlayOnSeqSelected));
			}
		));
		menu->addChild(createBoolPtrMenuItem("Play Input CV 0..10V", "", &module->bPlayUse10));
		menu->addChild(createBoolPtrMenuItem("Play Input selects sequence", "", &module->bSelectOnPlayInput));
		menu->addChild(createBoolPtrMenuItem("Copy after end of sequence", "", &module->bCopyOnEOS));
		menu->addChild(createBoolPtrMenuItem("Transpose after end of sequence", "", &module->bTransOnEOS));
		menu->addChild(createBoolPtrMenuItem("Set sequence after copy", "", &module->bSetSeqOnCopy));
	}

};

} //namespace pluginSSE

Model* modelKeySeq = createModel<pluginSSE::KeySeq, pluginSSE::KeySeqWidget>("KeySeq");
