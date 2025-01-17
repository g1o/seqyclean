/* 
 * File:   Report.h
 * Author: ilya
 *
 * Created on 8 Август 2012 г., 0:18
 */

#ifndef REPORT_H
#define	REPORT_H

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <math.h>
//#include <sparsehash/dense_hash_map>
#include "timer.h"
#include "util.h"
#include <streambuf>
#include <exception>
#include "Read.h"
#include <stdlib.h>
#include "sffreader.h"


//using google::dense_hash_map; 
using namespace std;

extern vector<Read*> reads;
//extern dense_hash_map<string, Read> reads_1; extern dense_hash_map<string, Read>::iterator it_reads_1;
//extern dense_hash_map<string, Read> reads_2; extern dense_hash_map<string, Read>::iterator it_reads_2;
//extern map<string, Read> reads_1; extern map<string, Read>::iterator it_reads_1;
//extern map<string, Read> reads_2; extern map<string, Read>::iterator it_reads_2;
extern vector<Read*> reads_1,reads_2;
extern bool qual_trim_flag;
//extern char *fastq_file;
/*Vectors : */
extern char *vector_file;
/*Contaminations : */
extern char *cont_file;
extern char* rlmids_file;
extern bool amplicon_flag;
extern bool pcr_flag;
extern long discard_counter;
extern bool output_sfffile_flag;
extern bool output_fastqfile_flag;
extern bool contaminants_flag;
extern bool vector_flag;
extern long illumina_pe_counter;
extern bool illumina_pe_flag;
extern bool lucy_only_flag;
extern map<int, string > vector_names;
extern unsigned short minimum_read_length;
extern string output_prefix;
extern fstream sum_stat;
extern string stat_str;
extern vector<char*> roche_names;
extern string roche_rep_file_name;
extern std::fstream sum_stat_tsv;
extern string version;
extern bool trim_adapters_flag;
extern short KMER_SIZE;
extern short DISTANCE;
extern unsigned short KMER_SIZE_CONT;
extern int max_al_mism;
extern string roche_output_file_name;
extern float max_a_error;
extern float max_e_at_ends;
extern unsigned short NUM_THREADS;
extern bool polyat_flag;

void MakeReport(string file_name);
void MakeLucyReport();
void MakeRocheReport(char* file_name, char* roche_fname); /*ROCHE clip points versus mine clip points*/


/*Writung out files*/
void WriteToFASTQIlluminaSE(char* file_name_1);
void WriteToSFFIllumina(char* file_name);
void CreateOutputFiles(string file_name);
void MakeLucyFastq(string custom_file_name);
void MakeLucyReport2(char *filename, vector<Read*>& reads);

#endif	/* REPORT_H */

