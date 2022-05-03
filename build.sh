TEST=${1-"foo"}


if [ $TEST == "-t" ];
then
    echo "Building tests..."
    g++ -W -Wall -Wshadow -pedantic -std=c++11 src/test.cpp src/ChatRoom.cpp src/UserInputHandling.cpp src/Communication.cpp src/ClientAttributes.cpp -o bin/tests -I ./inc
    echo "Done!"
fi

echo "Building server..."
g++ -W -Wall -Wshadow -pedantic -std=c++11 src/server.cpp src/ChatRoom.cpp src/Communication.cpp -o bin/server -I ./inc
echo "Done!"

echo "Building client..."
g++ -W -Wall -Wshadow -pedantic -std=c++11 src/client.cpp src/ChatRoom.cpp src/UserInputHandling.cpp src/Communication.cpp src/ClientAttributes.cpp -o bin/client -I ./inc
echo "Done!"
