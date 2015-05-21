/* *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare,
 *  Matthew Hausknecht, and the Reinforcement Learning and Artificial Intelligence 
 *  Laboratory
 * Released under the GNU General Public License; see License.txt for details. 
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 *  sharedLibraryInterfaceExample.cpp 
 *
 *  Sample code for running an agent with the shared library interface. 
 **************************************************************************** */

#include <iostream>
#include <ale_interface.hpp>
#include <games/RomUtils.hpp>
#ifdef __USE_SDL
  #include <SDL.h>
#endif

using namespace std;
class Features{
	public:
		virtual void getActiveFeaturesIndices(const ALEScreen &screen, const ALERAM &ram, vector<int>& features) = 0;
		void getCompleteFeatureVector(const ALEScreen &screen, const ALERAM &ram, vector<bool>& features);
		virtual int getNumberOfFeatures() = 0;
		virtual ~Features();
};
void Features::getCompleteFeatureVector(const ALEScreen &screen, const ALERAM &ram, vector<bool>& features){	
	assert(features.size() == 0); //If the vector is not empty this can be a mess
	//Get vector with active features:
	vector<int> temp;
	vector<int>& t = temp;
	this->getActiveFeaturesIndices(screen, ram, t);
	//Iterate over vector with all features storing the non-zero indices in the new vector:
	features = vector<bool>(this->getNumberOfFeatures(), 0);
	for(unsigned int i = 0; i < t.size(); i++){
		features[t[i]] = 1;
	}
}

Features::~Features(){}
class RAMFeatures : public Features::Features{
	private:
	public:
		RAMFeatures();
		void getActiveFeaturesIndices(const ALEScreen &screen, const ALERAM &ram, vector<int>& features);	
		int getNumberOfFeatures();
		~RAMFeatures();
};

#define BITS_IN_BYTE    8
#define BYTES_RAM     128
#define BITS_RAM     1024

typedef unsigned char byte_t;

RAMFeatures::RAMFeatures(){
}

void RAMFeatures::getActiveFeaturesIndices(
	const ALEScreen &screen, const ALERAM &ram, vector<int>& features){
	assert(features.size() == 0); //If the vector is not empty this can be a mess
	byte_t byte;
	char output[BITS_IN_BYTE];

	int pos = 0;
	for(int i = 0; i < BYTES_RAM; i++){
		//Decomposing byte in bits
		byte = ram.get(i);		
    	for (int b = 0; b < BITS_IN_BYTE; b++) {
  			output[b] = (byte >> b) & 1;
		}
    	//Saving bits in feature vector (little endian)
    	for(int b = 0; b < BITS_IN_BYTE; b++){
    		if(output[b]){
    			features.push_back(pos++);
    		}
    		else{
    			pos++;
    		}
    	}
	}
	//Bias:
	features.push_back(BITS_RAM);
}

int RAMFeatures::getNumberOfFeatures(){
	return BITS_RAM + 1;
}

RAMFeatures::~RAMFeatures(){}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " rom_file" << std::endl;
        return 1;
    }

    ALEInterface ale;

    // Get & Set the desired settings
    ale.setInt("random_seed", 123);

#ifdef __USE_SDL
    ale.setBool("display_screen", true);
    ale.setBool("sound", true);
#endif

    // Load the ROM file. (Also resets the system for new settings to
    // take effect.)
    ale.loadROM(argv[1]);

    cout<<readRam(&ale.theOSystem->console().system(),0)<<endl;
   
    ale.setMode(3);
    cout<<readRam(&ale.theOSystem->console().system(),0)<<endl;

    RAMFeatures features;
	vector<bool> F;
	F.clear();
	features.getCompleteFeatureVector(ale.getScreen(), ale.getRAM(), F);

    // Get the vector of legal actions
    ActionVect legal_actions = ale.getLegalActionSet();

    // Play 10 episodes
    for (int episode=0; episode<10; episode++) {
        float totalReward = 0;
        while (!ale.game_over()) {
            Action a = legal_actions[rand() % legal_actions.size()];
            // Apply the action and get the resulting reward
            float reward = ale.act(a);
            totalReward += reward;
            /*F.clear();
            features.getCompleteFeatureVector(ale.getScreen(), ale.getRAM(), F);
            for(int i = 0; i < 1024; i++){
                printf("%d ", (int) F[i]);
                if((i+1)%8 == 0){
                    printf("  ");
                }
                if((i+1)%32 == 0){
                    printf("\n");
                }
            }
            printf("\n");
            getchar();*/
        }
        cout << "Episode " << episode << " ended with score: " << totalReward << endl;
        ale.reset_game();
    }

    return 0;
}
