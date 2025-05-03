# filter_5_letter_words.py

with open("/usr/share/dict/words") as infile, open("words.txt", "w") as outfile:
    for word in infile:
        w = word.strip().lower()
        if len(w) == 5 and w.isalpha():
            outfile.write(w + "\n")
