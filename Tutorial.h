#include <string>
#include <vector>

class Tutorial
{
	bool initiated = false;

	std::string infoFrontal;
	std::string infoTemporal;
	std::string infoOccipital;
	std::string infoParietal;

	int stage = 0;
	void tutorialScript();
	
	int pickStage = 0;
	int pickMission = -1;
	void tutorialPickScript();
   std::vector<void*> images;

public:

	bool tutorialMode = false;

	void pickLobe(int id);
	void switchToTutorialMode();
	void draw2DtutorialGui();

   int getStage() { return stage; }

	Tutorial();
	~Tutorial();
};
