import glob, re

def main():
    with open("exp3_audio_visualizer.c", "r") as f:
        txt = f.read()

    txt = txt.replace("while (k <= j) {", "while (k >= 1 && k <= j) {")
    
    with open("exp3_audio_visualizer.c", "w") as f:
        f.write(txt)

if __name__ == "__main__":
    main()
