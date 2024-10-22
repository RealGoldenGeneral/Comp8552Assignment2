CC = g++
CXXFLAGS = -g --std=c++17 -isystem . -isystem glfw/include -Wno-narrowing
LINKFLAGS = -L glfw/lib -lglfw3 -lopengl32 -lgdi32

TARGET = c8552a2
SRCFILES = c8552qt.cpp glad.cpp

$(TARGET): $(SRCFILES)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(SRCFILES) $(LINKFLAGS)

clean:
	del /f $(TARGET).exe