#include <popt.h>
#include <math.h>
#include <vector>


#include "Features.hh"
#include "Song.hh"

int display_version;
int display_song_block;
int display_blocks;

const struct poptOption options[] =
    {
        {"version", 'V', POPT_ARG_NONE, &display_version, 0, "Display version and compile flags", NULL},
        {"song", 's', POPT_ARG_NONE, &display_song_block, 0, "Display Song feature block", NULL},
        {"blocks", 'b', POPT_ARG_NONE, &display_blocks, 0, "Display Feature blocks", NULL},
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    
void print_feature_group(FeatureGroup* group);

int main(int argc, const char *argv[])
{
    poptContext context = poptGetContext("viewer", argc, argv, options, 0);
    poptSetOtherOptionHelp(context, "[OPTION...] file.wav");
    display_version = 0;
    display_song_block = 0;
    display_blocks = 0;
    poptGetNextOpt(context);
    
    if (display_version) {
        std::cout << "Data Viewer: $Revision$" << std::endl;
        exit(EXIT_SUCCESS);
    }
    
    if (!(display_song_block ^ display_blocks)) {
        std::cout << "Please specify one of -s or -b" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    Song song(std::string("."), std::string(poptGetArg(context)));
    std::cout <<
    "\"Block\",\"Mean of Zero Crossing Rate\",\"Mean of First Order Autocorrelation\",\"Mean of Linear Regression\",\"Mean of Spectral Centroid\",\"Mean of Spectral Intensity\",\"Mean of Spectral Smoothness\",\"Mean of Spectral Spread\",\"Mean of Spectral Dissymmetry\",\"Variance of Zero Crossing Rate\",\"Variance of First Order Autocorrelation\",\"Variance of Linear Regression\",\"Variance of Spectral Centroid\",\"Variance of Spectral Intensity\",\"Variance of Spectral Smoothness\",\"Variance of Spectral Spread\",\"Variance of Spectral Dissymmetry\",\"Skewness of Zero Crossing Rate\",\"Skewness of First Order Autocorrelation\",\"Skewness of Linear Regression\",\"Skewness of Spectral Centroid\",\"Skewness of Spectral Intensity\",\"Skewness of Spectral Smoothness\",\"Skewness of Spectral Spread\",\"Skewness of Spectral Dissymmetry\",\"Kurtosis of Zero Crossing Rate\",\"Kurtosis of First Order Autocorrelation\",\"Kurtosis of Linear Regression\",\"Kurtosis of Spectral Centroid\",\"Kurtosis of Spectral Intensity\",\"Kurtosis of Spectral Smoothness\",\"Kurtosis of Spectral Spread\",\"Kurtosis of Spectral Dissymmetry\""
    << std::endl;
    
    if (display_song_block) {
        print_feature_group(song.song_features);
    } else {
        for (unsigned int i = 0; i < song.feature_blocks->size(); i++) {
            print_feature_group(song.feature_blocks->at(i));
        }
    }
}

int row_num = 1;
void print_feature_group(FeatureGroup* group)
{
    std::cout << row_num++ << ",";
    for (unsigned int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        if (!isnormal(group->mean[feature])) {
            std::cout << "\"NaN: " << fpclassify(group->mean[feature]) << "\",";
        } else {
            std::cout << group->mean[feature] << ",";
        }
    }
    for (unsigned int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
    
        if (!isnormal(group->variance[feature])) {
            std::cout << "\"NaN: " << fpclassify(group->variance[feature]) << "\",";
        } else {
            std::cout << group->variance[feature] << ",";
        }
    }
    for (unsigned int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        if (!isnormal(group->skewness[feature])) {
            std::cout << "\"NaN: " << fpclassify(group->skewness[feature]) << "\",";
        } else {
            std::cout << group->skewness[feature] << ",";
        }
    }
    for (unsigned int feature = 0; feature < NUMBER_OF_FEATURES; feature++) {
        if (!isnormal(group->kurtosis[feature])) {
            std::cout << "\"NaN: " << fpclassify(group->kurtosis[feature]) << "\"";
        } else {
            std::cout << group->kurtosis[feature];
        }
        if (feature < NUMBER_OF_FEATURES - 1) { std::cout << ","; }
    }
    std::cout << std::endl;
}
