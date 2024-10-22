CC = g++
CXXFLAGS = -g --std=c++17 -isystem . -isystem glfw/include -Wc++11-narrowing
LINKFLAGS = -L glfw/lib-macos -lglfw3 -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
TARGET = c8552a2
SRCFILES = c8552qt.cpp glad.cpp

$(TARGET): $(SRCFILES)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(SRCFILES) $(LINKFLAGS)

clean:
	\rm -f $(TARGET)

