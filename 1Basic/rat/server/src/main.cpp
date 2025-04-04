#include <iostream>

#include "Server.hpp"

const std::string HEADER =
    "                         .....             .....                          "
    "            ....    ...  \n"
    "                     .....   .....          ........                      "
    "                         \n"
    "                                    ........................              "
    "             ...         \n"
    "                          .....  .      ........-##.........           .  "
    "              ..         \n"
    "                         .:#=*..........=+%%##%%###%%#@#%*...........     "
    "              ..         \n"
    "                        ..*::-:.....-@*++++*##################@@#-....    "
    "         .               \n"
    "                       ...%%%%##%#=:-@#################*+*#########*...   "
    "         .               \n"
    "                     ..:%#######*-*-::###########+**+++++++###*++####+:.. "
    "       .                 \n"
    "     .    .        ..#@#########@*+--%##########+****+++######+++*#####@. "
    "                         \n"
    "             "
    ".....-@############################*+**##*########*++*#####+....          "
    "            \n"
    "        "
    "....=.--%%########################################################...     "
    "                 \n"
    "          "
    "..:+%######################################*+++##############+*#%:.       "
    "               \n"
    "          "
    ".*@@###%%#####%############+++*#########*+++++*######@######*++*#-....    "
    "               \n"
    "     ..   "
    ".:*##+*-:@#%#*#############*+++*#####*++++#*+######%########*++*#*......  "
    "               \n"
    ".....     "
    ".......:#@%+*##%#######*+*###########+++*##########%#######*+++*###......."
    ".........      \n"
    ".....     "
    "....-%#%%###%%########*++++##########*++*###########################%-...."
    "..=##+=......  \n"
    "        "
    ".....*##################*+++*###################*+*#@#################=---"
    "------=====++%-..\n"
    "        "
    "...=####################++++##############*####*++++##################-=#%"
    "+::............  \n"
    "        "
    "..#######***##@##################%######*++++#######*%##############%=...."
    "                 \n"
    "   "
    "......%######*++*@+.+#%##############%######*++**++#########@##########@:."
    "                      \n"
    "   "
    "..=++=---#%%*+=:.....%#############%%#######+++*#############@#####%%-...."
    "...                   \n"
    "  ...#**=+=*..... .  ..##++*########%%########################@@@%@%#%... "
    "   .                     \n"
    "  ...:#*@=.......  . .-%*+########@%###############%@@###%%@#--%+*%%....  "
    "  ... ..                 \n"
    "    .......      ....:########%%*...:++###*==:..:.................:..     "
    "                         \n"
    "  ...           ..-%#=-+###@+:............................ ..........     "
    "                         \n"
    "               .+@**++-**:.....                ..      .....              "
    "                         \n"
    "               ..-@@+#*.......    .                    .....              "
    "             ..          \n"
    "                ..........                ...                             "
    "                         \n"
    "               .....                      ...                             "
    "                         \n"
    "......           ...                      ...                             "
    "                         \n";

int main(int argc, char* argv[]) {
  system("clear");
  std::cout << HEADER << "\n\n";

  auto pServer = Server::getInstance();
  pServer->init(argv[1], atoi(argv[2]));
  pServer->loop();
  pServer->stop();
  return 0;
}