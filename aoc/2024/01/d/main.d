import std.algorithm;
import std.array;
import std.conv;
import std.exception: ErrnoException;
import std.math : abs;
import std.range : zip;
import std.stdio;
import std.typecons;

auto process_file(string path) {
    int[] lcol, rcol;
    int[int] freq, score;

    auto file = File(path);
    foreach (tokens; file.byLine().map!split) {
        assert(tokens.length == 2, "incorrect input file format");
        auto lnum = parse!int(tokens[0]);
        auto rnum = parse!int(tokens[1]);
        lcol ~= lnum;
        rcol ~= rnum;
        freq[lnum]++;
        score[rnum] += rnum;
    }
    lcol.sort!("a > b");
    rcol.sort!("a > b");

    int sum;
    foreach (ref l, ref r; zip(lcol, rcol)) {
        sum += abs(l - r);
    }
    int similarity;
    foreach (kvp; freq.byKeyValue) {
        similarity += kvp.value * score.get(kvp.key, 0);
    }
    return tuple(sum, similarity);
}

void main(string[] argv)
{
    foreach (string path; argv[1..$]) {
        try {
           auto res = process_file(path);
           writeln(path, ": sum=", res[0], ", similarity_score=", res[1]);
        }
        catch (ErrnoException e) {
            writeln(e.file, ": ", e.msg);
        }
    }
}
