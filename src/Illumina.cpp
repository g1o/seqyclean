#include "Illumina.h"

extern bool compressed_output;

/*i5 adapter*/
string tmpl_i5_1 = "AATGATACGGCGACCACCGAGATCTACAC";//"AGATCGGAAGAGCACACGTCTGAACTCCAGTCAC"; // "TACACTCTTTCCCTACACGACGCTCTTCCGATCT"
string tmpl_i5_2 = "ACACTCTTTCCCTACACGACGCTCTTCCGATCT"; 
 /*i7 adapter*/
string tmpl_i7_1 = "GATCGGAAGAGCACACGTCTGAACTCCAGTCAC";//"GTGACTGGAGTTCAGACGTGTGCTCTTCCGATCT";//"AGATCGGAAGAGCGTCGTGTAGGGAAAGAGTGTA";
string tmpl_i7_2 = "ATCTCGTATGCCGTCTTCTGCTTG";

extern string adapter_file;
extern bool custom_adapters;
vector<string> adapters;

// Статистика:
long long se_bases_kept, se_bases_discarded;
long long se_discard_cnt = 0;
long long se_bases_anal = 0;        
double avg_trim_len_se;

long long pe1_bases_anal, pe2_bases_anal, pe_bases_kept, pe_bases_discarded, se_pe1_bases_kept, se_pe2_bases_kept;
long long pe_discard_cnt;
double avg_trim_len_pe1, avg_trim_len_pe2;
/*Raw implementation of average. Later I will come with a better algorithm*/
long long avg_bases_pe1 = 0;
long long avg_bases_pe2 = 0;
long long avg_left_clip_1 = 0;
long long avg_left_clip_2 = 0;
long long avg_right_clip_1 = 0;
long long avg_right_clip_2 = 0;
long long cnt1_avg, cnt2_avg; //Counters needed for calculating the average trimming length
long long cnt_avg_len1, cnt_avg_len2; 
double avg_len_pe1, avg_len_pe2; 
double cnt_right_trim_pe1, avg_right_trim_len_pe1, cnt_right_trim_pe2, avg_right_trim_len_pe2; 
double cnt_left_trim_pe1, avg_left_trim_len_pe1, cnt_left_trim_pe2, avg_left_trim_len_pe2;
long long cnt1, cnt2;
long long pe_accept_cnt, se_pe1_accept_cnt, se_pe2_accept_cnt; 
long long ts_adapters1, ts_adapters2; 
long long num_vectors1, num_vectors2; 
long long num_contaminants1, num_contaminants2; 
long long accepted1,accepted2; 
long long discarded1,discarded2; 
long long perfect_ov_cnt, partial_ov_cnt; 
long long discarded_by_contaminant1, discarded_by_contaminant2; 
long long discarded_by_read_length1, discarded_by_read_length2; 
/*Left trims*/
long long left_trimmed_by_quality1 , left_trimmed_by_quality2; 
long long left_trimmed_by_vector1 , left_trimmed_by_vector2; 
/*Right trims/discards*/
long long right_trimmed_by_quality1 , right_trimmed_by_quality2; 
long long right_trimmed_by_adapter1 , right_trimmed_by_adapter2; 
long long right_trimmed_by_vector1 , right_trimmed_by_vector2;  
long long right_trimmed_by_polyat1, right_trimmed_by_polyat2; 
long long left_trimmed_by_polyat1, left_trimmed_by_polyat2; 
    
            
long long duplicates = 0;
        
std::string stat_str, tsv_stat_str;
   

//Dynamic Illumina: does not need space to store reads:
void IlluminaDynamic()
{
    cnt_avg_len1 = cnt_avg_len2 = 0;
    avg_len_pe1 = avg_len_pe2 = 0.0;
    cnt1_avg = cnt2_avg = 0;
    pe_bases_kept = pe_bases_discarded = se_pe1_bases_kept = se_pe2_bases_kept = 0;
    pe_discard_cnt = 0;
    pe1_bases_anal = pe2_bases_anal = 0;        
    avg_trim_len_pe1 = avg_trim_len_pe2 = 0;
    
    cnt_right_trim_pe1 = avg_right_trim_len_pe1 = cnt_right_trim_pe2 = avg_right_trim_len_pe2 = 0;
    cnt_left_trim_pe1 = avg_left_trim_len_pe1 = cnt_left_trim_pe2 = avg_left_trim_len_pe2 = 0;
    
    cnt1 = cnt2 = 0;
    pe_accept_cnt = se_pe1_accept_cnt = se_pe2_accept_cnt = 0;
    ts_adapters1 = ts_adapters2 = 0;
    num_vectors1 = num_vectors2 = 0;
    num_contaminants1 = num_contaminants2 = 0;
    accepted1 = accepted2 = 0;
    discarded1 = discarded2 = 0;
    perfect_ov_cnt = partial_ov_cnt = 0;
    discarded_by_contaminant1 = discarded_by_contaminant2 = 0;
    discarded_by_read_length1 = discarded_by_read_length2 = 0;
    left_trimmed_by_quality1 = left_trimmed_by_quality2 = 0;
    left_trimmed_by_vector1 = left_trimmed_by_vector2 = 0;
    right_trimmed_by_quality1 = right_trimmed_by_quality2 = 0;
    right_trimmed_by_adapter1 = right_trimmed_by_adapter2 = 0;
    right_trimmed_by_vector1 = right_trimmed_by_vector2 = 0;
    right_trimmed_by_polyat1 = right_trimmed_by_polyat2 = 0;
    left_trimmed_by_polyat1 = left_trimmed_by_polyat2 = 0;
    
    std::fstream rep_file1, rep_file2, pe_output_file1, pe_output_file2, shuffle_file, se_file, overlap_file;
    
    ogzstream pe_output_file1_gz, pe_output_file2_gz, shuffle_file_gz, se_file_gz, overlap_file_gz;
    
    if (detailed_report) {
        rep_file1.open(rep_file_name1.c_str(),ios::out);
        rep_file2.open(rep_file_name2.c_str(),ios::out);
    }
    
    if (detailed_report) {
        rep_file1 << "ReadID\tlclip\trclip\tTruSeq_pos\tRaw_read_length\tLlucy\tRlucy\tDiscarded\tContaminants\tVecStart\tVecEnd\tVecLen\n";
        rep_file2 << "ReadID\tlclip\trclip\tTruSeq_pos\tRaw_read_length\tLlucy\tRlucy\tDiscarded\tContaminants\tVecStart\tVecEnd\tVecLen\n";
    }
    std::cout << "Running the Illumina cleaning process...\n";
    sum_stat << "Running the Illumina cleaning process...\n";
    
    // Load adapters:
    LoadAdapters(adapter_file, custom_adapters);
    
    vector<std::string> record_block1, record_block2;
    
    if (!shuffle_flag)
    {
        if(compressed_output)
        {
            pe_output_file1_gz.open( pe_output_filename1.c_str() );
            pe_output_file2_gz.open( pe_output_filename2.c_str() ); 
        } else 
        {
            pe_output_file1.open( pe_output_filename1, std::ios::out );
            pe_output_file2.open( pe_output_filename2, std::ios::out );
        }
        
        
       
    } 
    else
    {
        if(compressed_output)
        {
            shuffle_file_gz.open( shuffle_filename.c_str() );
        }
        else
        {
            shuffle_file.open( shuffle_filename, std::ios::out );
        } 
    }

    if(compressed_output)
    {
        se_file_gz.open( se_filename.c_str());
    }
    
    else 
    {
        se_file.open( se_filename.c_str(), std::ios::out );
    }
    
    std::string st_str; //output statistics
    
    for(unsigned int jj=0; jj < pe1_names.size(); ++jj)
    {
        int ii = 0;
        std::string line1, line2;
        igzstream in1(/*fastq_file1*/pe1_names[jj]); //for R1
        igzstream in2(/*fastq_file2*/pe2_names[jj]); //for R2
        
        std::cout << "Processing files: " << pe1_names[jj] << ", " << pe2_names[jj] << "\n";
        sum_stat << "Processing files: " << pe1_names[jj] << ", " << pe2_names[jj] << "\n";
        
        while ( getline(in1,line1) && getline(in2,line2) )
        {
            /*Read ID*/
            if(ii==0) 
            {
                //Check for order
                std::vector <std::string> fields1, fields2;
                split_str( line1, fields1, " " );
                split_str( line2, fields2, " " );
                //cout << line1 << endl;
                if( (fields1[0] != fields2[0] ) && !old_style_illumina_flag)
                {
                    std::cout << "Warning: read IDs do not match in input files: PE1-> " << pe1_names[jj] << ", PE2-> " << pe2_names[jj] << endl;
                    sum_stat << "Warning: read IDs do not match in input files: PE1-> " << pe1_names[jj] << ", PE2-> " << pe2_names[jj] << endl;
                }
                        
                fields1.clear();
                fields2.clear();
                        
                if ( new2old_illumina && !old_style_illumina_flag ) //if convert to old-style illumina headers is true and not old illumina files.
                {
                    split_str( line1, fields1, " " );
                    line1 = fields1[0] + "#0/1";
                    fields1.clear();
                    fields2.clear();
                            
                    split_str( line2, fields1, " " );
                    line2 = fields1[0] + "#0/2";
                    fields1.clear();
                    fields2.clear();
                }
                        
                record_block1.push_back(line1); 
                record_block2.push_back(line2);
                        
                    ii++;
                    continue;
                }
                /*DNA string*/
                if(ii==1) 
                {
                    record_block1.push_back(line1);
                    record_block2.push_back(line2);
                    ii++;
                    continue;
                }
                /*a symbol "+"*/
                if(ii==2) 
                {
                    record_block1.push_back(line1);
                    record_block2.push_back(line2);
                    ii++;
                    continue;
                }
                if(ii==3) 
                {
                    cnt1+=1; cnt2+=1;
                    ii=0;
           
                    Read *read1 = new Read();
                    read1->illumina_readID = record_block1[0];
                    read1->initial_length = record_block1[1].length();
                    read1->read = record_block1[1];
                    read1->illumina_quality_string = line1;
                    pe1_bases_anal += (long long)read1->read.length();
                        
                    if(read1->initial_length <= minimum_read_length)
                    {
                        std::cout << "Warming: in PE1 file raw read length is less or equal then minimum_read_length\n" ; 
                        sum_stat << "Warming: in PE1 file raw read length is less or equal then minimum_read_length\n" ; 
                    }
                    
                    Read *read2 = new Read();
                    read2->illumina_readID = record_block2[0];
                    read2->initial_length = record_block2[1].length();
                    read2->read = record_block2[1];
                    read2->illumina_quality_string = line2;
                    pe2_bases_anal += static_cast<long long>(read2->read.length());
                        
                    if(read2->initial_length <= minimum_read_length)
                    {
                        std::cout << "Warming: in PE2 file, the raw read length is less or equal than minimum_read_length\n" ; 
                        sum_stat << "Warming: in PE2 file, the raw read length is less or equal than minimum_read_length\n" ; 
                    }
                    
                    // We have to make the length of the reads to be equal:
                    //unsigned int rlen = min(read1->read.length(), read2->read.length());
                    //read1->read = read1->read.substr(0,rlen);
                    //read2->read = read2->read.substr(0,rlen);
                        
                    //Serial realization - useful for debugging if something does not work as expected
                    //Duplicates removal
                    if(rem_dup)
                        screen_duplicates(read1, read2, duplicates);
                    //Checking for overlap:
                    bool overlap_found = false;
                    Read *c = new Read();
                    if(overlap_flag && (read1->discarded == 0) && (read2->discarded == 0)) {
                        Read *s1 = new Read();
                        Read *s2 = new Read(); 
                        int len = read1->read.length();
                        char t[len+1];
                        read1->read.copy(t, sizeof t);
                        t[len] = '\0';
                        s1->read = t; 
                        read1->illumina_quality_string.copy(t, sizeof t);
                        t[len] = '\0';
                        s1->illumina_quality_string = t;
                        read2->read.copy(t, sizeof t);
                        t[len] = '\0';
                        s2->read = t; 
                        read2->illumina_quality_string.copy(t, sizeof t);
                        t[len] = '\0';
                        s2->illumina_quality_string = t;
                        int ov;
                        std::string tread = MakeRevComplement(s2->read);
                        ov = find_overlap_pos(s1->read, tread, minoverlap);
                        if( ov > 0 ) {
                            partial_ov_cnt += 1;
                            //Overlap, make consensus sequence:
                            s1->read = s1->read.substr(0, ov);
                            s1->illumina_quality_string = s1->illumina_quality_string.substr(0, ov);
                            s2->read = MakeRevComplement(s2->read.substr(0,ov));
                            s2->illumina_quality_string = s2->illumina_quality_string.substr(0, ov);
                            reverse(s2->illumina_quality_string.begin(), s2->illumina_quality_string.end());
                            
                            c = make_consensus(s1, s2);
                            c->tru_sec_found = 1; c->tru_sec_pos = ov;
                            read1->tru_sec_found = 1; read2->tru_sec_found = 1;
                            read1->tru_sec_pos = ov-1; read2->tru_sec_pos = ov-1;
                            read1->merged = true; read2->merged = true;
                            overlap_found = true;
                         }
                              
                         if(overlap_found) {
                            IlluminaDynRoutine_post(c);
                            MakeClipPointsIllumina(c);
                             
                            if((c->rclip < c->lclip) || c->discarded) {
                               read1->discarded = 1;
                               read2->discarded = 1;
                               c->discarded = 1;
                            } else {
                                c->read = c->read.substr(0 , c->rclip );
                                c->illumina_quality_string = c->illumina_quality_string.substr(0,c->rclip) ; 
                                c->read = c->read.substr( c->lclip, c->rclip - c->lclip );
                                c->illumina_quality_string = c->illumina_quality_string.substr( c->lclip, c->rclip - c->lclip );
                            
                                // Read ID reconstruction:
                                vector<string> temp_id;
                                split_str( read1->illumina_readID, temp_id, " " );
                                vector<string> temp_id1;
                                split_str( temp_id[0], temp_id1, " " );
                                c->illumina_readID = temp_id1[0];
                                temp_id.clear();
                                temp_id1.clear();
                            }
                            
                        }
                        delete s1;
                        delete s2;
                    } 
                    
                    if(overlap_found && overlap_flag && (c->discarded == 0)) {
                        if(compressed_output) 
                        {
                            WriteSEFileGZ(se_file_gz, c);
                        } else 
                        { 
                            WriteSEFile(se_file, c);
                        }
                    } else {
                        // Удалить адаптеры, ошибочно считанные нуклеотиды и т.д.:
                        TrimIllumina(read1, read2);
                    }
                    
                    if( (read1->discarded == 0) && (read2->discarded == 0) && (read1->merged == false) && (read2->merged == false)) {
                        avg_trim_len_pe1 = (avg_trim_len_pe1*pe_accept_cnt + (read1->rclip - read1->lclip))/(pe_accept_cnt+1);
                        avg_trim_len_pe2 = (avg_trim_len_pe2*pe_accept_cnt + (read2->rclip - read2->lclip))/(pe_accept_cnt+1);
                        
                        if (!shuffle_flag) {
                            if(compressed_output)
                            {
                                WritePEFileGZ(pe_output_file1_gz, read1);
                                WritePEFileGZ(pe_output_file2_gz, read2);
                            }
                            else 
                            {
                                WritePEFile(pe_output_file1, read1);
                                WritePEFile(pe_output_file2, read2);
                            }
                            pe_bases_kept += static_cast<long long>(read1->read.length());
                            pe_bases_kept += static_cast<long long>(read2->read.length());
                            
                        } else {
                            if(compressed_output)
                            {
                                WriteShuffleFileGZ( shuffle_file_gz, read1, read2 );
                            }
                            else 
                            {
                                WriteShuffleFile( shuffle_file, read1, read2 );
                            }
                            
                            pe_bases_kept += static_cast<long long>(read1->read.length());
                            pe_bases_kept += static_cast<long long>(read2->read.length());
                            
                        }
                        pe_accept_cnt+=1;
                        /*       
                        cnt1_avg+=1;
                        avg_bases_pe1 += (int)read1->read.length();
                        avg_trim_len_pe1 = avg_bases_pe1/cnt1_avg;
                        cnt2_avg+=1;
                        avg_bases_pe2 += (int)read2->read.length();
                        avg_trim_len_pe2 = avg_bases_pe2/cnt2_avg;
                               
                        cnt_avg_len1 +=1; cnt_avg_len2 +=1;
                        */
                    } else if ((read1->discarded == 0) && (read2->discarded == 1)) {
                        if( new2old_illumina && !old_style_illumina_flag ) //if convert to old-style illumina headers is true and not old illumina files.
                            read1->illumina_readID = read1->illumina_readID.substr(0,read1->illumina_readID.length()-2);                                  
                                  
                        if(compressed_output)
                        {
                            WriteSEFileGZ( se_file_gz, read1 );
                        }
                        else 
                        {
                            WriteSEFile( se_file, read1 );
                        }
                        
                        se_pe1_accept_cnt+=1;
                        se_pe1_bases_kept += read1->read.length();
                        
                    } else if( (read1->discarded == 1) && (read2->discarded == 0) ) {
                        if( new2old_illumina && !old_style_illumina_flag ) //if convert to old-style illumina headers is true and not old illumina files.
                            read2->illumina_readID = read2->illumina_readID.substr(0,read2->illumina_readID.length()-3);                                  
                        
                        if(compressed_output)
                        {
                            WriteSEFileGZ( se_file_gz, read2 );
                        }
                        else 
                        {
                            WriteSEFile( se_file, read2 );
                        }
                        
                        se_pe2_accept_cnt +=1;
                        se_pe2_bases_kept += read2->read.length();
                     
                    } else if( (read1->merged == 0) && (read2->merged == 0) ) {
                        pe_discard_cnt+=1;
                        pe_bases_discarded += read1->read.length();
                        pe_bases_discarded += read2->read.length();
                    }
                        
                    if (detailed_report) {
                        rep_file1 << read1->illumina_readID.substr(1,read1->illumina_readID.length()-1) << "\t" << read1->lclip << "\t" << read1->rclip << "\t" << (read1->tru_sec_pos == -1 ? "NA" : int2str(read1->tru_sec_pos))  << "\t" << read1->initial_length << "\t" << (read1->lucy_lclip <= 1 ? 1 : read1->lucy_lclip) << "\t" << (read1->lucy_rclip <= 1 ? 1 : read1->lucy_rclip) << "\t" << read1->discarded << "\t" << read1->contaminants << "\t" << (vector_flag == true ? int2str(read1->v_start) : "NA") << "\t" << (vector_flag == true ? int2str(read1->v_end) : "NA") << "\t" << (vector_flag == true ? int2str(read1->vec_len) : "NA") << "\n";
                        rep_file2 << read2->illumina_readID.substr(1,read2->illumina_readID.length()-1) << "\t" << read2->lclip << "\t" << read2->rclip << "\t" << (read2->tru_sec_pos == -1 ? "NA" : int2str(read2->tru_sec_pos)) << "\t"  << read2->initial_length << "\t" << (read2->lucy_lclip <= 1 ? 1 : read2->lucy_lclip) << "\t" << (read2->lucy_rclip <= 1 ? 1 : read2->lucy_rclip) << "\t" << read2->discarded << "\t" << read2->contaminants << "\t" << (vector_flag == true ? int2str(read2->v_start) : "NA") << "\t" << (vector_flag == true ? int2str(read2->v_end) : "NA") << "\t" << (vector_flag == true ? int2str(read2->vec_len) : "NA") << "\n";
                    }
          
                    if (read1->tru_sec_found == 1) ts_adapters1++;
                    if (read1->vector_found == 1) num_vectors1++;
                    if (read1->contam_found == 1) num_contaminants1++;
                    if (read1->discarded == 0) accepted1++;
                    if (read1->discarded == 1) discarded1++;
                    if (read1->discarded_by_contaminant == 1) discarded_by_contaminant1++;
                    if (read1->discarded_by_read_length == 1) discarded_by_read_length1++;
                    if (read1->left_trimmed_by_quality == 1) left_trimmed_by_quality1++;
                    if (read1->left_trimmed_by_vector == 1) left_trimmed_by_vector1++;
                    if (read1->right_trimmed_by_quality == 1) right_trimmed_by_quality1++;
                    if (read1->right_trimmed_by_adapter == 1) right_trimmed_by_adapter1++;
                    if (read1->right_trimmed_by_vector == 1) right_trimmed_by_vector1++;
                    if (read1->right_trimmed_by_polyat == 1) right_trimmed_by_polyat1++;
                    if (read1->left_trimmed_by_polyat == 1) left_trimmed_by_polyat1++;
          
                    if (read2->tru_sec_found == 1) ts_adapters2++;
                    if (read2->vector_found == 1) num_vectors2++;
                    if (read2->contam_found == 1) num_contaminants2++;
                    if (read2->discarded == 0) accepted2++;
                    if (read2->discarded == 1) discarded2++;
                    if (read2->discarded_by_contaminant == 1) discarded_by_contaminant2++;
                    if (read2->discarded_by_read_length == 1) discarded_by_read_length2++;
                    if (read2->left_trimmed_by_quality == 1) left_trimmed_by_quality2++;
                    if (read2->left_trimmed_by_vector == 1) left_trimmed_by_vector2++;
                    if (read2->right_trimmed_by_quality == 1) right_trimmed_by_quality2++;
                    if (read2->right_trimmed_by_adapter == 1) right_trimmed_by_adapter2++;
                    if (read2->right_trimmed_by_vector == 1) right_trimmed_by_vector2++;
                    if (read2->right_trimmed_by_polyat == 1) right_trimmed_by_polyat2++;
                    if (read2->left_trimmed_by_polyat == 1) left_trimmed_by_polyat2++;
                        
                    record_block1.clear();
                    read1->illumina_readID.clear(); 
                    read1->illumina_quality_string.clear();
                    read1->read.clear();
          
                    record_block2.clear();
                    read2->illumina_readID.clear(); 
                    read2->illumina_quality_string.clear();
                    read2->read.clear();
          
                    delete read1;
                    delete read2;
                    delete c;
                        
                    if( ((cnt1 % 1000 ) == 0) && verbose) {
                        st_str = PrintIlluminaStatistics(cnt1, cnt2, 
                                    pe1_bases_anal, pe2_bases_anal, 
                                    ts_adapters1, ts_adapters2, 
                                    num_vectors1, num_vectors2, 
                                    num_contaminants1, num_contaminants2, 
                                    left_trimmed_by_quality1, left_trimmed_by_quality2,
                                    left_trimmed_by_vector1, left_trimmed_by_vector2, 
                                    avg_left_trim_len_pe1, avg_left_trim_len_pe2, 
                                    right_trimmed_by_adapter1, right_trimmed_by_adapter2, 
                                    right_trimmed_by_quality1,right_trimmed_by_quality2,
                                    right_trimmed_by_vector1,right_trimmed_by_vector2,
                                    avg_right_trim_len_pe1,avg_right_trim_len_pe2,
                                    discarded1, discarded2,
                                    discarded_by_contaminant1, discarded_by_contaminant2,
                                    discarded_by_read_length1, discarded_by_read_length2,
                                    pe_accept_cnt, pe_bases_kept, 
                                    pe_discard_cnt,pe_bases_discarded, 
                                    se_pe1_accept_cnt, se_pe1_bases_kept,
                                    se_pe2_accept_cnt, se_pe2_bases_kept,
                                    avg_trim_len_pe1, avg_trim_len_pe2,
                                    avg_len_pe1, avg_len_pe2,
                                    perfect_ov_cnt, partial_ov_cnt,
                                    duplicates,
                                    left_trimmed_by_polyat1, right_trimmed_by_polyat1,
                                    left_trimmed_by_polyat2, right_trimmed_by_polyat2
                                   );
                            
                        if (cnt1 > 1000) {
                            std::vector<std::string> t;
                            split_str(st_str, t, "\n");
                            for(unsigned int kk=0; kk < t.size(); ++kk){
                                   std::cout << "\033[A\033[2K";
                            }
                            t.clear();
                        }
                            
                        std::cout << st_str;
                            
                    }
                        
                    record_block1.clear();
                    record_block2.clear();
                    
          
                }
        }
        in1.close();
        in2.close();
        
        
    
    }
    
    
    st_str = PrintIlluminaStatistics(cnt1, cnt2, 
                            pe1_bases_anal, pe2_bases_anal, 
                            ts_adapters1, ts_adapters2, 
                            num_vectors1, num_vectors2, 
                            num_contaminants1, num_contaminants2, 
                            left_trimmed_by_quality1, left_trimmed_by_quality2,
                            left_trimmed_by_vector1, left_trimmed_by_vector2, 
                            avg_left_trim_len_pe1, avg_left_trim_len_pe2, 
                            right_trimmed_by_adapter1, right_trimmed_by_adapter2, 
                            right_trimmed_by_quality1,right_trimmed_by_quality2,
                            right_trimmed_by_vector1,right_trimmed_by_vector2,
                            avg_right_trim_len_pe1,avg_right_trim_len_pe2,
                            discarded1, discarded2,
                            discarded_by_contaminant1, discarded_by_contaminant2,
                            discarded_by_read_length1, discarded_by_read_length2,
                            pe_accept_cnt, pe_bases_kept, 
                            pe_discard_cnt,pe_bases_discarded, 
                            se_pe1_accept_cnt, se_pe1_bases_kept,
                            se_pe2_accept_cnt, se_pe2_bases_kept,
                            avg_trim_len_pe1, avg_trim_len_pe2,
                            avg_len_pe1, avg_len_pe2,
                            perfect_ov_cnt, partial_ov_cnt,
                            duplicates,
                            left_trimmed_by_polyat1, right_trimmed_by_polyat1,
                            left_trimmed_by_polyat2, right_trimmed_by_polyat2
                            
                            );
    
    
    if (verbose) {
        std::vector<std::string> t;
        split_str(st_str, t, "\n");
        for(unsigned int kk=0; kk < t.size()+1; ++kk)
        {
            std::cout << "\033[A\033[2K";
        }
        t.clear();
    }
    
    std::cout << st_str;
    
    sum_stat << st_str;
    
    
    sum_stat_tsv << PrintIlluminaStatisticsTSV(cnt1, cnt2, 
                            pe1_bases_anal, pe2_bases_anal, 
                            ts_adapters1, ts_adapters2, 
                            num_vectors1, num_vectors2, 
                            num_contaminants1, num_contaminants2, 
                            left_trimmed_by_quality1, left_trimmed_by_quality2,
                            left_trimmed_by_vector1, left_trimmed_by_vector2, 
                            avg_left_trim_len_pe1, avg_left_trim_len_pe2, 
                            right_trimmed_by_adapter1, right_trimmed_by_adapter2, 
                            right_trimmed_by_quality1,right_trimmed_by_quality2,
                            right_trimmed_by_vector1,right_trimmed_by_vector2,
                            avg_right_trim_len_pe1,avg_right_trim_len_pe2,
                            discarded1, discarded2,
                            discarded_by_contaminant1, discarded_by_contaminant2,
                            discarded_by_read_length1, discarded_by_read_length2,
                            pe_accept_cnt, pe_bases_kept, 
                            pe_discard_cnt,pe_bases_discarded, 
                            se_pe1_accept_cnt, se_pe1_bases_kept,
                            se_pe2_accept_cnt, se_pe2_bases_kept,
                            avg_trim_len_pe1, avg_trim_len_pe2,
                            avg_len_pe1, avg_len_pe2,
                            perfect_ov_cnt, partial_ov_cnt,
                            left_trimmed_by_polyat1, right_trimmed_by_polyat1,
                            right_trimmed_by_polyat2, right_trimmed_by_polyat2,
                            duplicates
                            ) 
                 << endl;
                
    std::cout << "====================Done cleaning====================\n";  
    sum_stat << "====================Done cleaning====================\n";  
    stat_str.clear();
    
    //Do not forget to close streams:
    rep_file1.close();
    rep_file2.close();
    pe_output_file1.close();
    pe_output_file2.close();
    shuffle_file.close();
    se_file.close();
    overlap_file.close();
    
    pe_output_file1_gz.close();
    pe_output_file2_gz.close();
    shuffle_file_gz.close();
    se_file_gz.close(); 
    overlap_file_gz.close();
    
}

int IlluminaDynRoutine(Read* read, bool& adapter_found, string &query_str)
{
    /*if((int)read->read.length() > minimum_read_length) {
        read->illumina_quality_string = read->illumina_quality_string.substr(0, read->read.length());
        read->clear_length = read->read.length();
    } else {
        read->discarded = 1;
        read->discarded_by_read_length = 1;
        return -1;
    }*/
    
    if(contaminants_flag) {
       if(CheckContaminants(read->read) == 0) {
           read->contam_found = 1;
           read->discarded_by_contaminant = 1;
           read->contaminants = 1;
           read->discarded = 1;
           return -1;
       }
    }
            
    //If quality trimming flag is set up -> perform the quality trimming before vector/contaminants/adaptors clipping.
    if(qual_trim_flag ) {
       QualTrimIllumina( read, max_a_error, max_e_at_ends );//This function generates LUCY clips of the read. Later they should be compared and read should be trimmed based on the results of comparison.
       
       if (read->discarded_by_quality == 1) {
          read->discarded = 1;
          
          return -1;
         
       }
    }
    
    if(polyat_flag)
        PolyAT_Trim(read); 
    
    if( vector_flag ) 
        CheckVector(read); 
       
    //First 15 bases of i5 adapter forward
    if(trim_adapters_flag && illumina_flag_se) {
        size_t found;
        if (!adapter_found){
            std::string ts_adapter = tmpl_i5_1.substr(0,15);
            found = read->read.find(ts_adapter);
            if( found != std::string::npos ){
                std::cout << "i5 adapter in forward first found in the read " << read->illumina_readID << ", in the position: " << found << "\n";
                sum_stat << "i5 adapter in forward first found in the read " << read->illumina_readID << ", in the position: " << found << "\n";
                adapter_found = true;
                query_str = ts_adapter;
                read->tru_sec_pos = found;
                read->tru_sec_found = 1;
            }else{
                //First 20 bases of i5 adapter in reverse complement
                ts_adapter = MakeRevComplement(tmpl_i5_2).substr(0,15);
                found = read->read.find( ts_adapter );
                if( found != string::npos ){
                    std::cout << "i5 adapter in forward first found in the read " << read->illumina_readID << ", in the position: " << found << "\n";
                    sum_stat << "i5 adapter in forward first found in the read " << read->illumina_readID << ", in the position: " << found << "\n";
                    adapter_found = true;
                    query_str = ts_adapter;
                    read->tru_sec_pos = found;
                    read->tru_sec_found = 1;
                } else {
                    //First 20 bases of i7 adapter forward
                    ts_adapter = tmpl_i7_1.substr(0,15);
                    found = read->read.find( ts_adapter );
                    if( found != std::string::npos ) {
                        cout << "i7 adapter in forward first found in the read " << read->illumina_readID << ", in the position: " << found << endl;
                        sum_stat << "i7 adapter in forward first found in the read " << read->illumina_readID << ", in the position: " << found << endl;
                        adapter_found = true;
                        query_str = ts_adapter;
                        read->tru_sec_pos = found;
                        read->tru_sec_found = 1;
                    } else {
                        //First 20 bases of i5 adapter in reverse complement
                        ts_adapter = MakeRevComplement(tmpl_i7_2).substr(0,15);
                        found = read->read.find( ts_adapter );
                        if( found != std::string::npos ) {
                            cout << "i7 adapter in forward first found in the read " << read->illumina_readID << ", in the position: " << found << endl;
                            sum_stat << "i7 adapter in forward first found in the read " << read->illumina_readID << ", in the position: " << found << endl;
                            adapter_found = true;
                            query_str = ts_adapter;
                            read->tru_sec_pos = found;
                            read->tru_sec_found = 1;
                        } else {
                            read->tru_sec_pos = -1;
                            read->tru_sec_found = 0;
                        }
                    }
                }
            }
        } else {
            bool adp_found = false;
            found = read->read.rfind( query_str );
            if( found != std::string::npos ) {
                adp_found = true;
                read->tru_sec_pos = found;
                read->tru_sec_found = 1;
            } else {
                //SSAHA job starts here
                iz_SSAHA *izssaha = new iz_SSAHA();
                AlignResult al_res = izssaha->Find( read->read , query_str );
                AlignScores scores;
                if( al_res.found_flag  ) {
                    scores = CalcScores(al_res.seq_1_al, al_res.seq_2_al, al_res.seq_1_al.length(), 0);
                    if(scores.mismatches <= max_al_mism  ) {
                        adp_found = true;
                        read->tru_sec_pos = al_res.pos;
                        read->tru_sec_found = 1;
                    }
                }
                delete izssaha;
            }
        
            if(!adp_found) {
                read->tru_sec_pos = -1;
                read->tru_sec_found = 0;
            }
        }
    }
    return 0;
}

void MakeClipPointsIllumina(Read* read) 
{
   //Clip points
   if( (qual_trim_flag ) && (vector_flag ) )
   {
        if(read->vector_found == 1)
        {
           if( read->v_start >= static_cast<int>(read->read.length() - read->v_end) ) //Vector is on the right side
           {
               //Lucy clip points are zero-based!
               
               read->lclip = read->lucy_lclip >= read->initial_length ? 0 : read->lucy_lclip;
               
               if(read->lclip > 0)
                  read->left_trimmed_by_quality = 1;
               
               
               read->rclip = min(trim_adapters_flag ? ( (read->tru_sec_pos == -1 || read->tru_sec_pos == 0) ? (int)read->read.length() : read->tru_sec_pos) : (int)read->read.length(), min(read->lucy_rclip+1, read->v_start) );
                    
               if( (read->rclip == read->lucy_rclip+1) && (read->rclip+1 <= read->initial_length ) )
               {
                 read->right_trimmed_by_quality = 1;
                 read->rclip = read->lucy_rclip;
               }
               else if((read->rclip == read->tru_sec_pos) && trim_adapters_flag)
               {
                 if( (read->rclip < static_cast<int>(read->read.length())) && (read->tru_sec_found == 1) && (read->rclip >= minimum_read_length))
                        read->right_trimmed_by_adapter = 1;
               }
               else if(read->rclip == read->v_start && (read->rclip < static_cast<int>(read->read.length())))
               {
                 read->right_trimmed_by_vector = 1;
               } else {
                   read->rclip = static_cast<int>(read->read.length());
               }
           }
           else //Vector is on the left side or the whole read is vector
           {
               //Keep in mind that Lucy's clip points are zero-based!
               
               read->lclip = max(read->lucy_lclip+1,read->v_end);//max(read->lucy_lclip,max(1, read->v_end ) );
           
               if(read->lclip >= read->initial_length)
                   read->lclip = 0;
               
               if( (read->lclip == read->lucy_lclip+1) && (read->lclip > 0) )//&& (read->lucy_lclip > 1)) 
               {
                 read->left_trimmed_by_quality = 1;
                 read->lclip = read->lucy_lclip;
               }
               if(read->lclip == read->v_end)
               {
                 read->left_trimmed_by_vector = 1;
               }
           
               read->rclip = min(trim_adapters_flag ? ((read->tru_sec_pos == -1 || read->tru_sec_pos == 0) ? static_cast<int>(read->read.length()) : read->tru_sec_pos) : static_cast<int>(read->read.length()), read->lucy_rclip+1 );
               
               if( (read->rclip == read->lucy_rclip+1) && (read->rclip+1 < read->initial_length ) )
               {
                 read->right_trimmed_by_quality = 1;
                 read->rclip = read->lucy_rclip;
               }
               else if( (read->rclip < static_cast<int>(read->read.length())) && (read->tru_sec_found == 1) && (read->rclip >= minimum_read_length) && trim_adapters_flag) {
                        read->right_trimmed_by_adapter = 1;
               } else {
                   read->rclip = static_cast<int>(read->read.length());
               }
           }
           
        } 
        else
        {
            //Keep in mind that Lucy's clip points are one-based!
            
            read->lclip = read->lucy_lclip;//max(read->lucy_lclip, 1);
            if((read->lclip > 0) && (read->lclip < read->initial_length)) {
                read->left_trimmed_by_quality = 1;
            } else {
                read->lclip = 0;
            }
            
            read->rclip = min(trim_adapters_flag ? ((read->tru_sec_pos == -1 || read->tru_sec_pos == 0) ? static_cast<int>(read->read.length()) : read->tru_sec_pos) : static_cast<int>(read->read.length()), read->lucy_rclip + 1 );
            
            if( (read->rclip == read->lucy_rclip+1) && (read->rclip < read->initial_length ) )
            {
              read->right_trimmed_by_quality = 1;
              read->rclip = read->lucy_rclip;
            }
            else if( (read->rclip < static_cast<int>(read->read.length())) && (read->tru_sec_found == 1) && (read->rclip >= minimum_read_length) && trim_adapters_flag) {
                        read->right_trimmed_by_adapter = 1;
            } else {
                read->rclip = static_cast<int>(read->read.length());
            }
        }
        
    } else if( (qual_trim_flag ) && (!vector_flag) ){
        //Keep in mind that Lucy's clip points are zero-based!
        read->lclip = read->lucy_lclip;
        if(read->lclip > 0)
           read->left_trimmed_by_quality = 1;
        
        if (trim_adapters_flag) {
            if (read->tru_sec_pos <= 0) {
                read->rclip = min(static_cast<int>(read->read.length()),read->lucy_rclip);
            } else {
                read->rclip = min(read->tru_sec_pos,read->lucy_rclip);
            }   
        } else {
            read->rclip = min(static_cast<int>(read->read.length()),read->lucy_rclip);
        }
        
        if((read->rclip == read->lucy_rclip) && (read->rclip < read->initial_length )){
            read->rclip = read->lucy_rclip;
            read->right_trimmed_by_quality = 1;
        } else if((read->rclip == read->tru_sec_pos) && trim_adapters_flag){
           if( (read->rclip < static_cast<int>(read->read.length())) && (read->tru_sec_found == 1) && (read->rclip >= minimum_read_length))
               read->right_trimmed_by_adapter = 1;
        }
    }
    else if( (!qual_trim_flag) && (vector_flag ) )
    {
       if( read->v_start >= (static_cast<int>(read->read.length()) - read->v_end) ) //Vector is on the right side
       {
          read->lclip = 0;
            
          read->rclip = min(trim_adapters_flag ? ((read->tru_sec_pos == -1 || read->tru_sec_pos == 0) ? static_cast<int>(read->read.length()) : read->tru_sec_pos) : static_cast<int>(read->read.length()), read->v_start == -1 ? read->tru_sec_pos : read->v_start );
                    
          if( (read->rclip == static_cast<unsigned short>(read->tru_sec_pos)) && trim_adapters_flag)
          {
             if( (read->rclip < static_cast<int>(read->read.length())) && (read->tru_sec_found == 1) && (read->rclip >= minimum_read_length))
                        read->right_trimmed_by_adapter = 1;
          }
          else if(read->rclip == read->v_start)
          {
             read->right_trimmed_by_vector = 1;
          }
           
       }
       else 
       {
          read->lclip = max(0, read->v_end == -1 ? 0 : read->v_end);
           
          if(read->lclip == read->v_end)
          {
             read->left_trimmed_by_vector = 1;
          }
           
          read->rclip = trim_adapters_flag ? ((read->tru_sec_pos == -1 || read->tru_sec_pos == 0) ? static_cast<int>(read->read.length()) : read->tru_sec_pos) : static_cast<int>(read->read.length());
          if( (read->rclip < static_cast<int>(read->read.length())) && (read->tru_sec_found == 1) && (read->rclip >= minimum_read_length) && trim_adapters_flag)
          {
                        read->right_trimmed_by_adapter = 1;
          }
       }
                
       if(read->rclip >= read->clear_length)
       {
          read->rclip = read->clear_length; read->right_trimmed_by_adapter = 0;
       }    
    }
    else if((!qual_trim_flag) && (!vector_flag))
    {
       read->rclip = trim_adapters_flag ? ( (read->tru_sec_pos == -1 || read->tru_sec_pos == 0) ? static_cast<int>(read->read.length()) : read->tru_sec_pos) : static_cast<int>(read->read.length());
       if( (read->rclip < static_cast<int>(read->read.length())) && (read->tru_sec_found == 1) && (read->rclip >= minimum_read_length) && trim_adapters_flag)
          read->right_trimmed_by_adapter = 1;
            
       read->lclip = 0;
    }
    
    
    if(polyat_flag) {
       if( (read->rclip > read->poly_A_clip) && (read->poly_A_found)) {
            if(read->rclip == read->tru_sec_pos) {
               read ->right_trimmed_by_adapter = 0;
            } else if(read->rclip == read->lucy_rclip) {
               read->right_trimmed_by_quality = 0;
            } else if(read->rclip == read->v_start) {
               read->right_trimmed_by_vector = 0;
            }
                    
            read->rclip = read->poly_A_clip;
            read->right_trimmed_by_polyat = 1;
        }
        if( (read->lclip < read->poly_T_clip) && (read->poly_T_found)) {
            if(read->lclip == read->lucy_lclip) {
                read->left_trimmed_by_quality = 0;
            } else if(read->lclip == read->v_end) {
                read->left_trimmed_by_vector = 0;
            } 
             
            read->lclip = read->poly_T_clip;
            read->left_trimmed_by_polyat = 1;
        }
   }
}

string New2OldNbl(string header)
{
    vector <string> fields1, fields2;
    
    split_str( header, fields1, " " );
    split_str( fields1[0], fields2, ":" );
    
    string tmp = string(fields2[0] + "_" + fields2[2] + ":" + fields2[3] + ":" + fields2[4] + ":" + fields2[5] + ":" + fields2[6] + "#0/" + fields1[1].substr(0,1) + " (" + header + ")");
    
    fields2.clear();
    fields1.clear(); 
    
    return tmp;
    
}

void WriteSEOverlap(fstream &overlap_file, Read *read)
{
    overlap_file << read->illumina_readID << "\n";
    overlap_file << read->read << "\n";
    overlap_file << '+' << "\n";
    overlap_file << read->illumina_quality_string << "\n";
}

void WriteSEOverlapGZ(ogzstream &overlap_file, Read *read)
{
    overlap_file << read->illumina_readID << "\n";
    overlap_file << read->read << "\n";
    overlap_file << '+' << "\n";
    overlap_file << read->illumina_quality_string << "\n";
}


void WritePEFile(fstream &pe_output_file, Read *read)
{
    pe_output_file << read->illumina_readID << "\n";
    pe_output_file << read->read << "\n";
    pe_output_file << '+' << "\n";
    pe_output_file << read->illumina_quality_string << "\n";
}

void WritePEFileGZ(ogzstream &pe_output_file, Read *read)
{
    pe_output_file << read->illumina_readID << "\n";
    pe_output_file << read->read << "\n";
    pe_output_file << '+' << "\n";
    pe_output_file << read->illumina_quality_string << "\n";
}

void WriteShuffleFile(fstream &shuffle_output_file, Read *read1, Read *read2)
{
    shuffle_output_file << read1->illumina_readID << "\n";
    shuffle_output_file << read1->read << "\n";
    shuffle_output_file << '+' << "\n";
    shuffle_output_file << read1->illumina_quality_string << "\n";
    
    shuffle_output_file << read2->illumina_readID << "\n";
    shuffle_output_file << read2->read << "\n";
    shuffle_output_file << '+' << "\n";
    shuffle_output_file << read2->illumina_quality_string << "\n";
}

void WriteShuffleFileGZ(ogzstream &shuffle_output_file, Read *read1, Read *read2)
{
    shuffle_output_file << read1->illumina_readID << "\n";
    shuffle_output_file << read1->read << "\n";
    shuffle_output_file << '+' << "\n";
    shuffle_output_file << read1->illumina_quality_string << "\n";
    
    shuffle_output_file << read2->illumina_readID << "\n";
    shuffle_output_file << read2->read << "\n";
    shuffle_output_file << '+' << "\n";
    shuffle_output_file << read2->illumina_quality_string << "\n";
}

void WriteSEFile(fstream &se_output_file, Read *read)
{
    se_output_file << read->illumina_readID << "\n";
    se_output_file << read->read << "\n";
    se_output_file << '+' << "\n";
    se_output_file << read->illumina_quality_string << "\n";
}


void WriteSEFileGZ(ogzstream &se_output_file, Read *read)
{
    se_output_file << read->illumina_readID << "\n";
    se_output_file << read->read << "\n";
    se_output_file << '+' << "\n";
    se_output_file << read->illumina_quality_string << "\n";
}

//Dynamic Illumina: does not need space to store reads:
void IlluminaDynamicSE()
{
    se_bases_kept = se_bases_discarded = 0;
    se_discard_cnt = 0;
    se_bases_anal = 0;        
    avg_trim_len_se = 0;
    /*Raw implementation of average. Later I will come with a better algorithm*/
    //long long avg_bases_se = 0;
    //long long avg_left_clip = 0;
    //long long avg_right_clip = 0;
    
    //long long cnt_avg; cnt_avg = 0; //Counters needed for calculating the average trimming length
    //long long cnt_avg_len; cnt_avg_len = 0;
                 
    double avg_len_se; avg_len_se = 0.0;
    double cnt_right_trim_se, avg_right_trim_len_se; 
    double cnt_left_trim_se, avg_left_trim_len_se;
    
    cnt_right_trim_se = avg_right_trim_len_se = 0;
    cnt_left_trim_se = avg_left_trim_len_se = 0;
    
    long long cnt; cnt = 0;
    long long se_accept_cnt; se_accept_cnt = 0;
    long long ts_adapters; ts_adapters = 0;
    long long num_vectors; num_vectors = 0;
    long long num_contaminants; num_contaminants = 0;
    long long accepted; accepted = 0;
    long long discarded; discarded = 0;
//    unsigned long discarded_by_quality1, discarded_by_quality2; discarded_by_quality1 = discarded_by_quality2 = 0;
    long long discarded_by_contaminant; discarded_by_contaminant = 0;
    long long discarded_by_read_length; discarded_by_read_length = 0;
//    unsigned long discarded_by_vector1 , discarded_by_vector2; discarded_by_vector1 = discarded_by_vector2 = 0;
    /*Left trims*/
    long long left_trimmed_by_quality; left_trimmed_by_quality = 0;
    long long left_trimmed_by_vector; left_trimmed_by_vector = 0;
    /*Right trims/discards*/
    long long right_trimmed_by_quality; right_trimmed_by_quality = 0;
    long long right_trimmed_by_adapter; right_trimmed_by_adapter = 0;
    long long right_trimmed_by_vector;  right_trimmed_by_vector = 0;
    
    long long right_trimmed_by_polyat, left_trimmed_by_polyat; right_trimmed_by_polyat = left_trimmed_by_polyat = 0;
    long long discarded_by_polyAT = 0;
    
    std::fstream rep_file, se_output_file;
    ogzstream se_output_file_gz;
    
    if (detailed_report) {
        rep_file.open(rep_file_name1.c_str(),std::ios::out);
        rep_file << "ReadID\tlclip\trclip\tTruSeq_pos\tTruSeq_type\tRaw_read_length\tLlucy\tRlucy\tDiscarded\tContaminants\tVectorID\tVecStart\tVecEnd\tVecLen\n";
    }
    std::cout << "Running the Illumina cleaning process...\n";
    sum_stat << "Running the Illumina cleaning process...\n";
    
    
    
    std::vector<std::string> record_block;
    
    
    if(compressed_output)
    {
        se_output_file_gz.open( se_output_filename.c_str());
    }
    else 
    {
        se_output_file.open( se_output_filename, std::ios::out );
    }
    
    std::string st_str;
    //int first_avg = 0;
    for(unsigned int jj=0; jj< se_names.size(); ++jj)
    {
        
        bool adapter_found = false;
        
        std::string query_string = "NA";
        
        int ii = 0;
        
        std::string line;
        igzstream in( se_names[jj] ); //for R1
        
        std::cout << "Processing files: " << se_names[jj] << "\n";
        sum_stat << "Processing files: " << se_names[jj] << "\n";
        
        while ( getline(in,line) )
        {
            /*Read ID*/
            if(ii==0) 
            {
                //Check for order
                    
                if ( new2old_illumina && !old_style_illumina_flag )
                {
                    vector <std::string> fields1, fields2;     
                    split_str( line, fields1, " " );
                    split_str( fields1[0], fields2, ":" );
                    if(fields2.size() >= 6)
                        line = std::string(fields2[0] + "_" + fields2[2] + ":" + fields2[3] + ":" + fields2[4] + ":" + fields2[5] + ":" + fields2[6] + "#0" ) ;
			
                    fields1.clear();
                    fields2.clear();
                }
                    
                record_block.push_back(line); 
                        
                ii++;
                continue;
            }
            /*DNA string*/
            if(ii==1) 
            {
                record_block.push_back(line); /*DNA string*/
                ii++;
                continue;
            }
            /*a symbol "+"*/
            if(ii==2) 
            {
                record_block.push_back(line);
                ii++;
                continue;
            }
            if(ii==3) 
            {
                ii=0;
           
                Read *read = new Read();
                read->illumina_readID = record_block[0];
                read->initial_length = record_block[1].length();
                read->read = record_block[1];
                read->illumina_quality_string = line;
                se_bases_anal += read->read.length();
                
                //Serial realization - useful for debugging if something does not work as expected
                if(rem_dup)
                    screen_duplicates(read, duplicates);
      
                        
                        
                IlluminaDynRoutine(read, adapter_found, query_string);
                
                if(read->discarded == 0)
                {
                   MakeClipPointsIllumina(read);
                }
                        
                cnt+=1;
          
                //Report
                if (detailed_report) {
                    rep_file << read->illumina_readID.substr(1,read->illumina_readID.length()-1) << "\t" << read->lclip << "\t" << read->rclip << "\t" << read->tru_sec_pos << "\t" << read->b_adapter << "\t" << read->initial_length << "\t" << read->lucy_lclip << "\t" << read->lucy_rclip << "\t" << read->discarded << "\t" << read->contaminants << "\t" << "NA" << "\n";
                }

                if( read->rclip >= static_cast<int>(read->read.length()) ) { read->rclip = read->read.length(); }
                if( read->lclip >= read->rclip ) { read->discarded = 1; read->discarded_by_read_length = 1; } 
                if( read->lclip >= static_cast<int>(read->read.length()) ) { read->discarded = 1; read->discarded_by_read_length = 1; }
                        
                if( static_cast<int>(read->read.length()) < minimum_read_length ) { read->discarded = 1; read->discarded_by_read_length = 1; }
                if( (read->rclip - read->lclip) < minimum_read_length ) { read->discarded = 1; read->discarded_by_read_length = 1; }
                    if( read->discarded == 0 )
                    {
                        avg_right_trim_len_se = (avg_right_trim_len_se*se_accept_cnt + (read->initial_length - read->rclip))/(se_accept_cnt+1);
                        avg_left_trim_len_se = (avg_left_trim_len_se*se_accept_cnt + read->lclip)/(se_accept_cnt+1);
                        avg_trim_len_se = (avg_trim_len_se*se_accept_cnt + (read->rclip - read->lclip))/(se_accept_cnt+1);
                        se_accept_cnt+=1;
                          
                        read->read = read->read.substr(0 , read->rclip );
                        read->illumina_quality_string = read->illumina_quality_string.substr(0,read->rclip) ; 
                        read->read = read->read.substr( read->lclip, read->rclip - read->lclip );
                        read->illumina_quality_string = read->illumina_quality_string.substr( read->lclip, read->rclip - read->lclip );
                        
                        if(compressed_output)
                        {
                            WriteSEFileGZ(se_output_file_gz, read);
                        }
                        else 
                        {
                            WriteSEFile(se_output_file, read);
                        }
                          
                        se_bases_kept += read->read.length();
                    } 
                        
                    if (read->tru_sec_found == 1) ts_adapters++;
                    if (read->vector_found == 1) num_vectors++;
                    if (read->contam_found == 1) num_contaminants++;
                    if (read->discarded == 0) accepted++;
                    if (read->discarded == 1) discarded++;
                    if (read->discarded_by_contaminant == 1) discarded_by_contaminant++;
                    if (read->discarded_by_read_length == 1) discarded_by_read_length++;
                    if (read->left_trimmed_by_quality == 1) left_trimmed_by_quality++;
                    if (read->left_trimmed_by_vector == 1) left_trimmed_by_vector++;
                    if (read->right_trimmed_by_quality == 1) right_trimmed_by_quality++;
                    if (read->right_trimmed_by_adapter == 1) right_trimmed_by_adapter++;
                    if (read->right_trimmed_by_vector == 1) right_trimmed_by_vector++;
                    if (read->right_trimmed_by_polyat == 1) right_trimmed_by_polyat++;
                    if (read->left_trimmed_by_polyat == 1)  left_trimmed_by_polyat++;
                    if(read->discarded_by_polyAT == 1) discarded_by_polyAT++;
                        
                    record_block.clear();
                    read->illumina_readID.clear(); 
                    read->illumina_quality_string.clear();
                    read->read.clear();
          
                    delete read;
                        
                    if( ((cnt % 1000 ) == 0) && verbose)
                    {
                        st_str = PrintIlluminaStatisticsSE(cnt, 
                                    se_bases_anal, 
                                    ts_adapters,
                                    num_vectors,
                                    num_contaminants, 
                                    left_trimmed_by_quality,
                                    left_trimmed_by_vector, 
                                    avg_left_trim_len_se,
                                    right_trimmed_by_adapter,
                                    right_trimmed_by_quality,
                                    right_trimmed_by_vector,
                                    avg_right_trim_len_se,
                                    discarded, 
                                    discarded_by_contaminant,
                                    discarded_by_read_length,
                                    se_accept_cnt, se_bases_kept, 
                                    se_discard_cnt,se_bases_discarded, 
                                    avg_trim_len_se,
                                    avg_len_se,
                                    right_trimmed_by_polyat,
                                    left_trimmed_by_polyat,
                                    discarded_by_polyAT,
                                    duplicates
                                   );
                            
                        if (cnt > 1000)
                        {
                            std::vector<std::string> t;
                            split_str(st_str, t, "\n");
                            for(unsigned int kk=0; kk < t.size(); ++kk)
                            {
                                std::cout << "\033[A\033[2K";
                            }
                            t.clear();
                        }
                            
                    std::cout << st_str;
                     
                }
            }
        }
        
        in.close();
        
    }
    
    
    st_str = PrintIlluminaStatisticsSE(cnt, 
                                    se_bases_anal, 
                                    ts_adapters,
                                    num_vectors,
                                    num_contaminants, 
                                    left_trimmed_by_quality,
                                    left_trimmed_by_vector, 
                                    avg_left_trim_len_se,
                                    right_trimmed_by_adapter,
                                    right_trimmed_by_quality,
                                    right_trimmed_by_vector,
                                    avg_right_trim_len_se,
                                    discarded, 
                                    discarded_by_contaminant,
                                    discarded_by_read_length,
                                    se_accept_cnt, se_bases_kept, 
                                    se_discard_cnt,se_bases_discarded, 
                                    avg_trim_len_se,
                                    avg_len_se,
                                    left_trimmed_by_polyat, 
                                    right_trimmed_by_polyat,
                                    discarded_by_polyAT,
                                    duplicates
                                   );
    
    if (verbose) {
        std::vector<std::string> t;
        split_str(st_str, t, "\n");
        for(unsigned int kk=0; kk < t.size(); ++kk)
        {
            std::cout << "\033[A\033[2K";
            sum_stat << "\033[A\033[2K";
        }
        t.clear();
    }
    
    std::cout << st_str;
    sum_stat << st_str;
    
    sum_stat_tsv << PrintIlluminaStatisticsTSVSE(cnt,
                                    se_bases_anal, 
                                    ts_adapters, 
                                    num_vectors,  
                                    num_contaminants, 
                                    left_trimmed_by_quality, 
                                    left_trimmed_by_vector, 
                                    avg_left_trim_len_se, 
                                    right_trimmed_by_adapter, 
                                    right_trimmed_by_quality,
                                    right_trimmed_by_vector,
                                    avg_right_trim_len_se,
                                    discarded, 
                                    discarded_by_contaminant, 
                                    discarded_by_read_length,
                                    se_accept_cnt, 
                                   avg_trim_len_se,
                                   left_trimmed_by_polyat, 
                                   right_trimmed_by_polyat,
                                   discarded_by_polyAT,
                                   duplicates
                            ) << endl;
                 
    
    cout << "====================Done cleaning====================\n";  
    sum_stat << "====================Done cleaning====================\n";  
    
    se_output_file_gz.close();
    se_output_file.close();
    
    if (detailed_report)
        rep_file.close();
   
}


string PrintIlluminaStatistics(long long cnt1, long long cnt2, 
                                    long long  pe1_bases_anal, long long  pe2_bases_anal, 
                                    long long ts_adapters1, long long ts_adapters2, 
                                    long long num_vectors1, long long num_vectors2, 
                                    long long num_contaminants1, long long num_contaminants2, 
                                    long long left_trimmed_by_quality1, long long left_trimmed_by_quality2,
                                    long long left_trimmed_by_vector1, long long left_trimmed_by_vector2, 
                                    double avg_left_trim_len_pe1, double avg_left_trim_len_pe2, 
                                    long long right_trimmed_by_adapter1, long long right_trimmed_by_adapter2, 
                                    long long right_trimmed_by_quality1,long long right_trimmed_by_quality2,
                                    long long right_trimmed_by_vector1,long long right_trimmed_by_vector2,
                                    double avg_right_trim_len_pe1,double avg_right_trim_len_pe2,
                                    long long discarded1, long long discarded2,
                                    long long discarded_by_contaminant1, long long discarded_by_contaminant2,
                                    long long discarded_by_read_length1, long long discarded_by_read_length2,
                                    long long pe_accept_cnt, long long  pe_bases_kept, 
                                    long long pe_discard_cnt,long long  pe_bases_discarded, 
                                    long long se_pe1_accept_cnt, long long se_pe1_bases_kept,
                                    long long se_pe2_accept_cnt, long long se_pe2_bases_kept,
                                    double avg_trim_len_pe1, double avg_trim_len_pe2,
                                    double avg_len_pe1, double avg_len_pe2,
                                    long long perfect_ov_cnt, long long partial_ov_cnt,
                                    long long duplicates,
                                    long long left_trimmed_by_polyat1, long long right_trimmed_by_polyat1,
                                    long long left_trimmed_by_polyat2, long long right_trimmed_by_polyat2
                                    )
{
    
     std::string stat_str = std::string("====================Summary Statistics====================\n");
     stat_str += std::string("PE1 reads analyzed: ") +  std::to_string(cnt1) + string(", Bases: ") +  std::to_string(pe1_bases_anal) + string("\n")  +
                        (vector_flag ? "# of reads with vector: " + std::to_string(num_vectors1) + ", " + std::to_string( (double)num_vectors1/(double)cnt1*100.0) + "%\n" : "") +
                        ((qual_trim_flag || vector_flag || polyat_flag) ? "Reads left trimmed ->\n" : "" ) +
                        (qual_trim_flag ? "By quality: " +  std::to_string(left_trimmed_by_quality1) + "\n" : "" ) +
                        (vector_flag ? "By vector: " +  std::to_string(left_trimmed_by_vector1) + "\n" : "" ) +
                        (polyat_flag ? "By poly A/T: " + std::to_string(left_trimmed_by_polyat1) + "\n" : "") +
                        ((qual_trim_flag || vector_flag || polyat_flag) ? "Average left trim length: " + std::to_string(avg_left_trim_len_pe1) + " bp\n" : "") +
                        ((trim_adapters_flag || qual_trim_flag || vector_flag || polyat_flag) ? "Reads right trimmed ->\n" : "") +
                        (trim_adapters_flag ? "By adapter: " +  std::to_string(right_trimmed_by_adapter1) + "\n" : "") +
                        (qual_trim_flag ? "By quality: " +  std::to_string(right_trimmed_by_quality1) + "\n" : "") +
                        (vector_flag ? "By vector: " +  std::to_string(right_trimmed_by_vector1) + "\n" : "" ) +
                        (polyat_flag ? "By poly A/T: " + std::to_string(right_trimmed_by_polyat1) + "\n" : "") +
                        ((trim_adapters_flag || qual_trim_flag || vector_flag || polyat_flag) ? "Average right trim length: " + std::to_string(avg_right_trim_len_pe1) + " bp\n" : "") +
                        "PE1 reads discarded: " + std::to_string(discarded1) + "\n" +
                        "-----------------------------------------------------------\n" +
                        "PE2 reads analyzed: " + std::to_string(cnt2) + ", Bases: " + std::to_string(pe2_bases_anal) + "\n" +
                        (vector_flag ? ("# of reads with vector: " + std::to_string(num_vectors2) + ", " + std::to_string( (double)num_vectors2/(double)cnt2*100.0) + "%\n") : "") +
                        ((qual_trim_flag || vector_flag || polyat_flag) ? "Reads left trimmed ->\n" : "" ) +
                        (qual_trim_flag ? "By quality: " +  std::to_string(left_trimmed_by_quality2) + "\n" : "" ) +
                        (vector_flag ? "By vector: " +  std::to_string(left_trimmed_by_vector2) + "\n" : "" ) +
                        (polyat_flag ? "By poly A/T: " + int2str(left_trimmed_by_polyat2) + "\n" : "") +
                        ((qual_trim_flag || vector_flag || polyat_flag) ? "Average left trim length: " + std::to_string(avg_left_trim_len_pe2) + " bp\n" : "") +
                        ((trim_adapters_flag || qual_trim_flag || vector_flag || polyat_flag) ? "Reads right trimmed ->\n" : "") +
                        (trim_adapters_flag ? "By adapter: " +  std::to_string(right_trimmed_by_adapter2) + "\n" : "") +
                        (qual_trim_flag ? "By quality: " +  std::to_string(right_trimmed_by_quality2) + "\n" : "") +
                        (vector_flag ? "By vector: " +  std::to_string(right_trimmed_by_vector2) + "\n" : "" ) +
                        (polyat_flag ? "By poly A/T: " + std::to_string(right_trimmed_by_polyat2) + "\n" : "") +                      
                        ((trim_adapters_flag || qual_trim_flag || vector_flag || polyat_flag) ? "Average right trim length: " + std::to_string(avg_right_trim_len_pe2) + " bp\n" : "") +
                        "PE2 reads discarded:" + std::to_string(discarded2) + "\n" +
                        "----------------------Summary for PE & SE----------------------\n" +
                        ("Pairs kept: " + std::to_string(pe_accept_cnt) + ", " + std::to_string( (double)pe_accept_cnt/(double)cnt1*100.0) + "%, Bases: " + std::to_string(pe_bases_kept) + ", " + std::to_string( (double)pe_bases_kept/(double)(pe1_bases_anal+pe2_bases_anal)*100) +  "%\n") +
                        ("Pairs discarded: " + std::to_string(pe_discard_cnt) + ", " + std::to_string( (double)pe_discard_cnt/(double)cnt1*100.0) + "%, Bases: " + std::to_string(pe_bases_discarded) + ", " + std::to_string( (double)pe_bases_discarded/(double)(pe1_bases_anal+pe2_bases_anal)*100) +  "%\n") +
                        (contaminants_flag ? "Contaminated pairs: " +  std::to_string(discarded_by_contaminant2) + "\n" : "" ) +
                        ("Single Reads PE1 kept: " + std::to_string(se_pe1_accept_cnt) + ", Bases: " + std::to_string(se_pe1_bases_kept) +"\n") +
                        ("Single Reads PE2 kept: " + std::to_string(se_pe2_accept_cnt) + ", Bases: " + std::to_string(se_pe2_bases_kept) +"\n") +
                        ("Average trimmed length PE1: " + std::to_string(avg_trim_len_pe1) + " bp\n") +
                        ("Average trimmed length PE2: " + std::to_string(avg_trim_len_pe2) + " bp\n") +
                        (trim_adapters_flag ? "Adapters: " + std::to_string(ts_adapters2) + ", " + std::to_string( (double)ts_adapters2/(double)cnt2*100.0) + "%\n" : "") +
                        (overlap_flag ? "Overlaps: " + std::to_string(partial_ov_cnt) + ", "+ std::to_string( (double)partial_ov_cnt/(double)cnt1*100.0) + "%\n" : "") + 
                        (rem_dup ? "Duplicates: " + std::to_string(duplicates) + "\n" : "");
                        
     return stat_str;
    
}

string PrintIlluminaStatisticsTSV(long long cnt1, long long cnt2, 
                                    long long  pe1_bases_anal, long long  pe2_bases_anal, 
                                    long long ts_adapters1, long long ts_adapters2, 
                                    long long num_vectors1, long long num_vectors2, 
                                    long long num_contaminants1, long long num_contaminants2, 
                                    long long left_trimmed_by_quality1, long long left_trimmed_by_quality2,
                                    long long left_trimmed_by_vector1, long long left_trimmed_by_vector2, 
                                    double avg_left_trim_len_pe1, double avg_left_trim_len_pe2, 
                                    long long right_trimmed_by_adapter1, long long right_trimmed_by_adapter2, 
                                    long long right_trimmed_by_quality1,long long right_trimmed_by_quality2,
                                    long long right_trimmed_by_vector1,long long right_trimmed_by_vector2,
                                    double avg_right_trim_len_pe1,double avg_right_trim_len_pe2,
                                    long long discarded1, long long discarded2,
                                    long long discarded_by_contaminant1, long long discarded_by_contaminant2,
                                    long long discarded_by_read_length1, long long discarded_by_read_length2,
                                    long long pe_accept_cnt, long long pe_bases_kept, 
                                    long long pe_discard_cnt,long long pe_bases_discarded, 
                                    long long se_pe1_accept_cnt, long long se_pe1_bases_kept,
                                    long long se_pe2_accept_cnt, long long se_pe2_bases_kept,
                                    double avg_trim_len_pe1, double avg_trim_len_pe2,
                                    double avg_len_pe1, double avg_len_pe2,
                                    long long perfect_ov_cnt, long long partial_ov_cnt,
                                    long long left_trimmed_by_polyat1, long long right_trimmed_by_polyat1,
                                    long long left_trimmed_by_polyat2, long long right_trimmed_by_polyat2,
                                    long long duplicates
                                    )
{
    
    std::string filename_str;
    
    for(unsigned int i=0; i < pe1_names.size(); ++i)
    {
        filename_str += std::string(pe1_names[i]) + ", " + std::string(pe2_names[i]);
    }
        
    std::string stat_str_tsv =   version + "\t" + 
                                filename_str + "\t" +
                                (trim_adapters_flag  ? "YES" : "NO") + "\t" +
                                ( vector_flag ? "YES" : "NO") + "\t" +
                                ( vector_flag ? int2str(KMER_SIZE) : "NA") + "\t" +
                                ( vector_flag ? int2str(DISTANCE) : "NA") +"\t" +
                                ( contaminants_flag ? "YES" : "NO") +"\t" +
                                ( contaminants_flag ? int2str(KMER_SIZE_CONT) : "NA") +"\t" +
                                (qual_trim_flag ? "YES" : "NO") +"\t" +
                                (qual_trim_flag ? double2str(-10*log10(max_a_error)) : "NA")+ "\t" +
                                (qual_trim_flag ? double2str(-10*log10(max_e_at_ends)) : "NA")+ "\t" +
                                output_prefix +"\t" +
                                rep_file_name1+ "\t" +
                                rep_file_name2 +"\t" +
                                ( !shuffle_flag ?  pe_output_filename1 : "NA" ) +"\t"+
                                ( !shuffle_flag ?  pe_output_filename2 : "NA") +"\t"+
                                ( shuffle_flag ? shuffle_filename : "NA" ) +"\t" +
                                se_filename+ "\t" +
                                int2str(max_al_mism) +"\t" +
                                int2str(minimum_read_length)+ "\t" +
                                ( new2old_illumina ? "YES" : "NO") + "\t"; 
                   
    
    
    stat_str_tsv += std::to_string(cnt1) + "\t" + std::to_string(pe1_bases_anal) + "\t"  +
                    std::to_string(ts_adapters1) + "\t" + std::to_string( (double)ts_adapters1/(double)cnt1*100.0) + "\t" + 
                    ( vector_flag ? std::to_string(num_vectors1) + "\t" + std::to_string( (double)num_vectors1/(double)cnt1*100.0) + "\t" : "NA\tNA\t" ) +
                    ( contaminants_flag ? std::to_string(num_contaminants1) + "\t" + std::to_string( (double)num_contaminants1/(double)cnt1*100.0) + "\t" : "NA\tNA\t" ) +
                    ( qual_trim_flag ? std::to_string(left_trimmed_by_quality1) + "\t" : "NA\t" ) +
                    ( vector_flag ? std::to_string(left_trimmed_by_vector1) + "\t" : "NA\t" ) +
                    std::to_string(avg_left_trim_len_pe1) + "\t" +
                    std::to_string(right_trimmed_by_adapter1) + "\t" +
                    ( qual_trim_flag ? std::to_string(right_trimmed_by_quality1) + "\t" : "NA\t") +
                    ( vector_flag ? std::to_string(right_trimmed_by_vector1) + "\t" : "NA\t" ) +
                    std::to_string(avg_right_trim_len_pe1) + "\t" +
                    std::to_string(discarded1) + "\t" +
                    ( contaminants_flag ? std::to_string(discarded_by_contaminant1) + "\t" : "NA\t" ) +
                    std::to_string(discarded_by_read_length1) + "\t" +
                    std::to_string(cnt2) + "\t" + std::to_string(pe2_bases_anal) + "\t" +
                    std::to_string(ts_adapters2) + "\t" + std::to_string( (double)ts_adapters2/(double)cnt2*100.0) + "\t" +
                    ( vector_flag ? (std::to_string(num_vectors2) + "\t" + std::to_string( (double)num_vectors2/(double)cnt2*100.0) + "\t") : "NA\tNA\t") +
                    ( contaminants_flag? std::to_string(num_contaminants2) + "\t" + std::to_string( (double)num_contaminants2/(double)cnt2*100.0) + "\t" : "NA\tNA\t" ) +
                    (qual_trim_flag ? std::to_string(left_trimmed_by_quality2) + "\t" : "NA\t" ) +
                    ( vector_flag ? std::to_string(left_trimmed_by_vector2) + "\t" : "NA\t" ) +
                    std::to_string(avg_left_trim_len_pe2) + "\t"  +
                    ( qual_trim_flag ? std::to_string(right_trimmed_by_quality2) + "\t" : "NA\t") +
                    (vector_flag ? std::to_string(right_trimmed_by_vector2) + "\t" : "NA\t" ) +
                    std::to_string(right_trimmed_by_adapter2) + "\t" +
                    std::to_string(avg_right_trim_len_pe2) + "\t" +
                    std::to_string(discarded2) + "\t" +
                    (contaminants_flag ? std::to_string(discarded_by_contaminant2) + "\t" : "NA\t" ) +
                    std::to_string(discarded_by_read_length2) + "\t" + 
                    (std::to_string(pe_accept_cnt) + "\t" + std::to_string( (double)pe_accept_cnt/(double)cnt1*100.0) + "\t" + std::to_string(pe_bases_kept) + "\t" + std::to_string( (double)pe_bases_kept/(double)(pe1_bases_anal+pe2_bases_anal)*100) +  "\t") +
                    ( std::to_string(pe_discard_cnt) + "\t" + std::to_string( (double)pe_discard_cnt/(double)cnt1*100.0) + "\t" + std::to_string(pe_bases_discarded) + "\t" + std::to_string( (double)pe_bases_discarded/(double)(pe1_bases_anal+pe2_bases_anal)*100) +  "\t") +
                    (std::to_string(se_pe1_accept_cnt) + "\t" + std::to_string(se_pe1_bases_kept) +"\t") +
                    (std::to_string(se_pe2_accept_cnt) + "\t" + std::to_string(se_pe2_bases_kept) +"\t") +
                    (std::to_string(avg_trim_len_pe1) + "\t") +
                    std::to_string(avg_trim_len_pe2) +
                    ( overlap_flag ? "\t" + std::to_string(perfect_ov_cnt) + "\t" + std::to_string(partial_ov_cnt) : "\tNA\tNA") +
                    (polyat_flag ? "\tYES\t" + std::to_string(cdna) + "\t" + std::to_string(c_err) + "\t" + std::to_string(crng) + "\t" + std::to_string(left_trimmed_by_polyat1) + "\t" + std::to_string(right_trimmed_by_polyat1) + "\t" + std::to_string(left_trimmed_by_polyat2) + "\t" + std::to_string(right_trimmed_by_polyat2) : "\tNA\tNA\tNA\tNA\tNA\tNA\tNA\tNA") +
                    ( rem_dup ? "\tYES\t" + std::to_string(duplicates) + "\t" + std::to_string(size_dw) + "\t" + std::to_string(start_dw) + "\t" + std::to_string(max_dup) + "\n" : "\tNA\tNA\tNA\tNA\tNA\n");
                      
    return stat_str_tsv;
}

string PrintIlluminaStatisticsSE(long long cnt, long long se_bases_anal, 
                                    long long ts_adapters,
                                    long long num_vectors,
                                    long long num_contaminants, 
                                    long long left_trimmed_by_quality,
                                    long long left_trimmed_by_vector, 
                                    double avg_left_trim_len_se,
                                    long long right_trimmed_by_adapter,
                                    long long right_trimmed_by_quality,
                                    long long right_trimmed_by_vector,
                                    double avg_right_trim_len_se,
                                    long long discarded, 
                                    long long discarded_by_contaminant,
                                    long long discarded_by_read_length,
                                    long long se_accept_cnt, long long se_bases_kept, 
                                    long long se_discard_cnt,long long se_bases_discarded, 
                                    double avg_trim_len_se,
                                    double avg_len_se,
                                    long long left_trimmed_by_polyat, long long right_trimmed_by_polyat,
                                    long long discarded_by_polyAT,
                                    long long duplicates
                                    )
{
    
   
    
    std::string ans = "====================Summary Statistics====================\n" +
                        ("SE reads analyzed: " +  std::to_string(cnt)  + ", Bases:" +  std::to_string(se_bases_anal)  + "\n") +
                        "Found ->\n" +
                        (trim_adapters_flag ? "Adapters: " + std::to_string(ts_adapters) + ", " + std::to_string( (double)ts_adapters/(double)cnt*100.0) + "%\n" : "") + 
                        (vector_flag ? "# of reads with vector: " + std::to_string(num_vectors) + ", " + std::to_string( (double)num_vectors/(double)cnt*100.0) + "%\n" : "") +
                        (contaminants_flag ? "# of reads with contaminants: " + std::to_string(num_contaminants) + ", " + std::to_string( (double)num_contaminants/(double)cnt*100.0) + "%\n" : "") +
                        ((qual_trim_flag || vector_flag) ? "Reads left trimmed ->\n" : "" ) +
                        (qual_trim_flag ? "By quality: " +  std::to_string(left_trimmed_by_quality) + "\n" : "" ) +
                        (vector_flag ? "By vector: " +  std::to_string(left_trimmed_by_vector) + "\n" : "" ) +
                        (polyat_flag ? "By poly A/T: " + std::to_string(left_trimmed_by_polyat) + "\n": "") +
                        "Average left trim length: " + std::to_string(avg_left_trim_len_se) + " bp\n" +
                        "Reads right trimmed ->\n" +
                        (trim_adapters_flag ? "By adapter: " +  std::to_string(right_trimmed_by_adapter) + "\n": "") +
                        (qual_trim_flag ? "By quality: " +  std::to_string(right_trimmed_by_quality) + "\n" : "") +
                        (vector_flag ? "By vector: " +  std::to_string(right_trimmed_by_vector) + "\n" : "" ) +
                        (polyat_flag ? "By poly A/T: " + std::to_string(right_trimmed_by_polyat) + "\n" : "") +
                        "Average right trim length: " + std::to_string(avg_right_trim_len_se) + " bp\n" +
                        "SE reads discarded: " + std::to_string(discarded) + "\n" +
                        ( contaminants_flag ? "By contaminants: " +  std::to_string(discarded_by_contaminant) + "\n" : "" ) +
                        "By read length: " +  std::to_string(discarded_by_read_length) + "\n" +
                        "----------------------Summary for SE----------------------\n" +
                        ("Reads kept: " + std::to_string(se_accept_cnt) + ", " + std::to_string( (double)se_accept_cnt/(double)cnt*100.0) + "%, Bases: " + std::to_string(se_bases_kept) + ", " + std::to_string( (double)se_bases_kept/(double)(se_bases_anal)*100) +  "%\n") +
                        ("Average trimmed length: " + std::to_string(avg_trim_len_se) + " bp\n") +
                        ( rem_dup ? "Duplicates: " + std::to_string(duplicates) + "\n" : "");
    
    return ans;
   
}


std::string PrintIlluminaStatisticsTSVSE(long long cnt,
                                    long long se_bases_anal, 
                                    long long ts_adapters, 
                                    long long num_vectors,  
                                    long long num_contaminants, 
                                    long long left_trimmed_by_quality, 

                                    long long left_trimmed_by_vector, 
                                    double avg_left_trim_len_se, 
                                    long long right_trimmed_by_adapter, 
                                    long long right_trimmed_by_quality,
                                    long long right_trimmed_by_vector,
                                    double avg_right_trim_len_se,
                                    long long discarded, 
                                    long long discarded_by_contaminant, 
                                    long long discarded_by_read_length,
                                    long long se_accept_cnt, 
                                    double avg_trim_len_se,
                                    long long left_trimmed_by_polyat, long long right_trimmed_by_polyat,
                                    long long discarded_by_polyAT,
                                    long long duplicates
                                    )
{
    
    std::string filename_str;
    
    for(unsigned int i=0; i < pe1_names.size(); ++i)
    {
        filename_str += std::string(se_names[i]);
    }
        
    std::string stat_str_tsv =  version + "\t" + 
                                filename_str + "\t" +
                                (trim_adapters_flag  ? "YES" : "NO") + "\t" +
                                ( vector_flag ? "YES" : "NO") + "\t" +
                                ( vector_flag ? std::to_string(KMER_SIZE) : "NA") + "\t" +
                                ( vector_flag ? std::to_string(DISTANCE) : "NA") +"\t" +
                                ( contaminants_flag ? "YES" : "NO") +"\t" +
                                ( contaminants_flag ? std::to_string(KMER_SIZE_CONT) : "NA") +"\t" +
                                (qual_trim_flag ? "YES" : "NO") +"\t" +
                                (qual_trim_flag ? std::to_string(-10*log10(max_a_error)) : "NA")+ "\t" +
                                (qual_trim_flag ? std::to_string(-10*log10(max_e_at_ends)) : "NA")+ "\t" +
                                output_prefix +"\t" +
                                rep_file_name1+ "\t" +
                                ( !shuffle_flag ?  se_output_filename : "NA" ) +"\t"+
                                std::to_string(max_al_mism) +"\t" +
                                std::to_string(minimum_read_length)+ "\t" +
                                ( new2old_illumina ? "YES" : "NO") + "\t";
                                
                   
        stat_str_tsv += std::to_string(cnt)   + "\t" + //reads analyzed
                        std::to_string(se_bases_anal) + "\t"  + //bases
                        std::to_string(ts_adapters) + "\t" + //adapters
                        std::to_string( (double)ts_adapters/(double)cnt*100.0) + "\t" + //perc adapters
                        ( vector_flag ? std::to_string(num_vectors) + "\t" + std::to_string( (double)num_vectors/(double)cnt*100.0) + "\t" : "NA\tNA\t" ) + //perc vectors
                        ( contaminants_flag ? std::to_string(num_contaminants) + "\t" + //cont
                        std::to_string( (double)num_contaminants/(double)cnt*100.0) + "\t" : "NA\tNA\t" ) + //perc cont
                        ( qual_trim_flag ? std::to_string(left_trimmed_by_quality) + "\t" : "NA\t" ) +  //left trimmed qual
                        ( vector_flag ? std::to_string(left_trimmed_by_vector) + "\t" : "NA\t" ) + //left trimmed vect
                        std::to_string(avg_left_trim_len_se) + "\t" + //avg left trim len
                        std::to_string(right_trimmed_by_adapter) + "\t" + 
                        ( qual_trim_flag ? std::to_string(right_trimmed_by_quality) + "\t" : "NA\t") +
                        ( vector_flag ? std::to_string(right_trimmed_by_vector) + "\t" : "NA\t" ) +
                        std::to_string(avg_right_trim_len_se) + "\t" +
                        std::to_string(discarded) + "\t" + //discard
                        ( contaminants_flag ? std::to_string(discarded_by_contaminant) + "\t" : "NA\t" ) +
                        std::to_string(discarded_by_read_length) + "\t" +
                        std::to_string(se_accept_cnt) + "\t" + //se reads kept
                        std::to_string( (double)se_accept_cnt/(double)cnt*100.0) + "\t" + //perc kept
                        std::to_string(se_bases_kept) + "\t" + //bases kept
                        std::to_string( (double)se_bases_kept/(double)se_bases_anal*100.0) + "\t" + //%
                        std::to_string(avg_trim_len_se) +
                        (polyat_flag ? "\tYES\t" + std::to_string(cdna) + "\t" + std::to_string(c_err) + "\t" + std::to_string(crng) + "\t" + std::to_string(left_trimmed_by_polyat) + "\t" + std::to_string(right_trimmed_by_polyat) : "\tNA\tNA\tNA\tNA\tNA\tNA") +
                        ( rem_dup ? "\tYES\t" + std::to_string(duplicates) + "\t" + std::to_string(size_dw) + "\t" + std::to_string(start_dw) + "\t" + std::to_string(max_dup) + "\n" : "\tNA\tNA\tNA\tNA\tNA\n");
    
     return stat_str_tsv;
    
    
}

void RemoveContaminants(vector<Read*>& illumina_reads)
{
    
    for(unsigned int index = 0; index < illumina_reads.size(); index++)
    {
        if(contaminants_flag ) 
        {
            try 
            {//Check against contaminants :
                if(CheckContaminants(illumina_reads[index]->read) == 0) 
                {
                    illumina_reads[index]->discarded_by_contaminant = 1;
                    illumina_reads[index]->contaminants = 1;
                    illumina_reads[index]->discarded = 1;
                                
                }
            }
            catch(std::exception& e)
            {
                std::cout << e.what() << "\n";
            }
        }
    }
}

void ClearNNs( vector<Read*>& reads ) 
{
    /*Clear NNs*/
    for(unsigned int ii = 0; ii < reads.size(); ii++) 
    {
        size_t found_Ns = ((reads[ii]->read).substr(0,reads[ii]->initial_length-100)).find("NNN");
        if( found_Ns != string::npos ) {
            /*If the read is discarded, its pair must be discarded also:*/
            reads[ii]->discarded = 1;
            continue;
         } else {
            for(int s=0; s<10; s++) 
            {
                TrimNs(reads[ii]->read);
                reads[ii]->illumina_quality_string = reads[ii]->illumina_quality_string.substr(0, reads[ii]->read.length());
                reads[ii]->clear_length = reads[ii]->read.length();
            }
         }
    }
}

int IlluminaDynRoutine_post(Read* read)
{
    if(polyat_flag) {
       //If poly A/T flag is set:
       PolyAT_Trim(read);
    }
    
    if(contaminants_flag )
    {
       if(CheckContaminants(read->read) == 0) 
       {
           read->contam_found = 1;
           read->discarded_by_contaminant = 1;
           read->contaminants = 1;
           read->discarded = 1;
           return -1;
       }
    }
            
        
    //If quality trimming flag is set up -> perform the quality trimming before vector/contaminants/adaptors clipping.
    if(qual_trim_flag) {
        QualTrimIllumina( read, max_a_error, max_e_at_ends );//This function generates LUCY clips of the read. Later they should be compared and read should be trimmed based on the results of comparison.
        if (read->discarded_by_quality == 1) {
            read->discarded = 1;
            return -1;
       }
    }
    
    if( vector_flag ) 
        CheckVector(read); 
    
    return 0;
}


// Сначала - адаптеры, затем - ошибки
void TrimAdapterSE(Read* read) {
    bool adapter_found = false;
    for (unsigned int i=0; i<adapters.size() && !adapter_found; i++) {
        iz_SSAHA *izssaha = new iz_SSAHA();
        string ts_adapter = adapters[i];
        AlignResult al_res = izssaha->Find( read->read , ts_adapter );
        if( al_res.found_flag  ) {
            AlignScores scores = CalcScores(al_res.seq_1_al, al_res.seq_2_al, al_res.seq_1_al.length(), 0);
            if(scores.mismatches <= max_al_mism  ) {
               read->tru_sec_pos = al_res.pos;
               read->tru_sec_found = 1;
               adapter_found = true;
               if (read->tru_sec_pos + adapters[i].length() > 0.5*read->read.length()) {
                    read->lclip = 0; read->rclip = read->tru_sec_pos;
                    read->read = read->read.substr(0, read->rclip);
                    read->illumina_quality_string = read->illumina_quality_string.substr(0, read->rclip);
                } else {
                    read->lclip = read->tru_sec_pos+adapters[i].length(); read->rclip = read->read.length();
                    read->read = read->read.substr(read->lclip, read->read.length() - read->lclip);
                    read->illumina_quality_string = read->illumina_quality_string.substr(read->lclip, read->illumina_quality_string.length() - read->lclip);
                }
            }
        }
        delete izssaha;
    }
}

int TrimIllumina(Read* read1, Read* read2)
{
    int tmp_avg_right_clip_1, tmp_avg_left_clip_1;
    int tmp_avg_right_clip_2, tmp_avg_left_clip_2;
    
    read1->lclip = 0; read1->rclip = read1->read.length();
    read2->lclip = 0; read2->rclip = read2->read.length();
    
    tmp_avg_right_clip_1 = read1->initial_length - read1->rclip;
    tmp_avg_left_clip_1 = read1->lclip;
    
    tmp_avg_right_clip_2 = read2->initial_length - read2->rclip;
    tmp_avg_left_clip_2 = read2->lclip;
    
    if(contaminants_flag )
    {
       if(CheckContaminants(read1->read) == 0) 
       {
           read1->contam_found = 1;
           read1->discarded_by_contaminant = 1;
           read1->contaminants = 1;
           read1->discarded = 1;
           
           read2->contam_found = 1;
           read2->discarded_by_contaminant = 1;
           read2->contaminants = 1;
           read2->discarded = 1;
           
           return -1;
       }
       if(CheckContaminants(read2->read) == 0) 
       {
           read1->contam_found = 1;
           read1->discarded_by_contaminant = 1;
           read1->contaminants = 1;
           read1->discarded = 1;
           
           read2->contam_found = 1;
           read2->discarded_by_contaminant = 1;
           read2->contaminants = 1;
           read2->discarded = 1;
           
           return -1;
       }
    }
    
    // Trim adapters
    bool adapter_found = false;
    if (trim_adapters_flag) {
        adapter_found = TrimAdapterPE(read1,read2);
    }
    
    // Обновляем статистику:
    tmp_avg_right_clip_1 = read1->initial_length - read1->rclip;
    tmp_avg_left_clip_1 = read1->lclip;
    
    tmp_avg_right_clip_2 = read2->initial_length - read2->rclip;
    tmp_avg_left_clip_2 = read2->lclip;
    
    // Trim quality
    //If quality trimming flag is set up -> perform the quality trimming before vector/contaminants/adaptors clipping.
    if( qual_trim_flag  ) {
        // Сбрасываем точки усечения:
        read1->lclip = 0; read1->rclip = read1->read.length();
        read2->lclip = 0; read2->rclip = read2->read.length(); 
        QualTrimIllumina( read1, max_a_error, max_e_at_ends );//This function generates LUCY clips of the read. Later they should be compared and read should be trimmed based on the results of comparison.
        if (read1->discarded_by_quality == 1)
        {
            read1->discarded = 1;
            read1->lclip = read1->rclip = 1;
        } else {
            if (read1->lucy_lclip > 0) {
                read1->lclip = read1->lucy_lclip;
                read1->left_trimmed_by_quality = 1;
            }
            if (read1->lucy_rclip < read1->rclip) {
                read1->rclip = read1->lucy_rclip;
                read1->right_trimmed_by_quality = 1;
            }
           
            tmp_avg_left_clip_1 += read1->lclip;
            tmp_avg_right_clip_1 += read1->read.length() - read1->rclip;
           
            read1->read = read1->read.substr(0, read1->rclip);
            read1->illumina_quality_string = read1->illumina_quality_string.substr(0, read1->rclip);
            //
            read1->read = read1->read.substr(read1->lclip, read1->read.length()-read1->lclip);
            read1->illumina_quality_string = read1->illumina_quality_string.substr(read1->lclip, read1->illumina_quality_string.length()-read1->lclip);
           
            read1->lucy_lclip =0;
            read1->lucy_rclip = read1->read.length();
       
        }
       
        QualTrimIllumina( read2, max_a_error, max_e_at_ends );//This function generates LUCY clips of the read. Later they should be compared and read should be trimmed based on the results of comparison.
        if (read2->discarded_by_quality == 1)
        {
            read2->discarded = 1;
            read2->lclip = read2->rclip = 1;
        } else {
            if (read2->lucy_lclip > 0) {
                read2->lclip = read2->lucy_lclip;
                read2->left_trimmed_by_quality = 1;
            }
            if (read2->lucy_rclip < read2->rclip) {
                read2->rclip = read2->lucy_rclip;
                read2->right_trimmed_by_quality = 1;
            }
           
            tmp_avg_left_clip_2 += read2->lclip;
            tmp_avg_right_clip_2 += read2->read.length() - read2->rclip;
           
            read2->read = read2->read.substr(0, read2->rclip);
            read2->illumina_quality_string = read2->illumina_quality_string.substr(0, read2->rclip);
           
            read2->read = read2->read.substr(read2->lclip, read2->read.length()-read2->lclip);
            read2->illumina_quality_string = read2->illumina_quality_string.substr(read2->lclip, read2->illumina_quality_string.length()-read2->lclip);
           
            read2->lucy_lclip =0;
            read2->lucy_rclip = read2->read.length();
        }
    }
        
    if( vector_flag ) {
        // Сбрасываем точки усечения:
        read1->lclip = 0; read1->rclip = read1->read.length();
        read2->lclip = 0; read2->rclip = read2->read.length(); 
       
        CheckVector(read1);
        CheckVector(read2);
        
        if(read1->vector_found == 1) {
            if( read1->v_start >= static_cast<int>(read1->read.length() - read1->v_end) ) { //Vector is on the right side
                //Lucy clip points are zero-based!
                read1->rclip = read1->v_start;
                read1->right_trimmed_by_vector = 1;
            }
            else //Vector is on the left side or the whole read is vector
            {
                read1->lclip = read1->v_end;//max(read->lucy_lclip,max(1, read->v_end ) );
                read1->left_trimmed_by_vector = 1;
            }
        }
        
        if(read2->vector_found == 1) {
            if( read2->v_start >= static_cast<int>(read2->read.length() - read2->v_end) ) { //Vector is on the right side
                //Lucy clip points are zero-based!
                read2->rclip = read2->v_start;
                read2->right_trimmed_by_vector = 1;
            }
            else //Vector is on the left side or the whole read is vector
            {
                read2->lclip = read2->v_end;//max(read->lucy_lclip,max(1, read->v_end ) );
                read2->left_trimmed_by_vector = 1;
            }
        }
        
        tmp_avg_left_clip_1 += read1->lclip;
        tmp_avg_right_clip_1 += read1->read.length() - read1->rclip;
        tmp_avg_left_clip_2 += read2->lclip;
        tmp_avg_right_clip_2 += read2->read.length() - read2->rclip;
        
    }
    
    if(polyat_flag) {
        
        // Сбрасываем точки усечения:
        read1->lclip = 0; read1->rclip = read1->read.length();
        read2->lclip = 0; read2->rclip = read2->read.length(); 
       
        //Trim poly A/T:
        PolyAT_Trim(read1); 
        PolyAT_Trim(read2);
       
        if( (read1->rclip > read1->poly_A_clip) && (read1->poly_A_found)) {
            read1->rclip = read1->poly_A_clip;
            read1->right_trimmed_by_polyat = 1;
        }
        if( (read1->lclip < read1->poly_T_clip) && (read1->poly_T_found)) {
            read1->lclip = read1->poly_T_clip;
            read1->left_trimmed_by_polyat = 1;
        }
       
        if( (read2->rclip > read2->poly_A_clip) && (read2->poly_A_found)) {
            read2->rclip = read2->poly_A_clip;
            read2->right_trimmed_by_polyat = 1;
        }
        if( (read2->lclip < read2->poly_T_clip) && (read2->poly_T_found)) {
            read2->lclip = read2->poly_T_clip;
            read2->left_trimmed_by_polyat = 1;
        }
       
        tmp_avg_left_clip_1 += read1->lclip;
        tmp_avg_right_clip_1 += read1->read.length() - read1->rclip;
        tmp_avg_left_clip_2 += read2->lclip;
        tmp_avg_right_clip_2 += read2->read.length() - read2->rclip;
    }
    
    // Проверяем на минимальную длину:
    if (read1->rclip - read1->lclip < minimum_read_length) {
	read1->discarded = 1;
        read1->discarded_by_read_length = 1;
    }
    if (read2->rclip - read2->lclip < minimum_read_length) {
	read2->discarded = 1;
        read2->discarded_by_read_length = 1;
    }
    
    avg_right_trim_len_pe1 = (avg_right_trim_len_pe1*cnt_right_trim_pe1 + tmp_avg_right_clip_1)/(cnt_right_trim_pe1+1);
    avg_left_trim_len_pe1 = (avg_left_trim_len_pe1*cnt_left_trim_pe1 + tmp_avg_left_clip_1)/(cnt_left_trim_pe1+1);
    
    avg_right_trim_len_pe2 = (avg_right_trim_len_pe2*cnt_right_trim_pe2 + tmp_avg_right_clip_2)/(cnt_right_trim_pe2+1);
    avg_left_trim_len_pe2 = (avg_left_trim_len_pe2*cnt_left_trim_pe2 + tmp_avg_left_clip_2)/(cnt_left_trim_pe2+1);
    
    cnt_right_trim_pe1 += 1;cnt_left_trim_pe1 += 1;
    cnt_right_trim_pe2 += 1;cnt_left_trim_pe2 += 1;
    
    return 0;
}

bool TrimAdapterPE(Read *read1, Read *read2) {
    
    std::string revcomp = MakeRevComplement(read2->read);
    int o = find_overlap_pos(read1->read, revcomp, adapterlength);
    if( (o > 0) && (o != -10000)) {
        read1->tru_sec_found = 1; read2->tru_sec_found = 1;
        read1->tru_sec_pos = o-1;//o; 
        read2->tru_sec_pos = o-1;//o;
       
        read1->read = read1->read.substr(0, read1->tru_sec_pos);
        read1->illumina_quality_string = read1->illumina_quality_string.substr(0, read1->tru_sec_pos);
       
        read2->read = read2->read.substr(0, read2->tru_sec_pos);
        read2->illumina_quality_string = read2->illumina_quality_string.substr(0, read2->tru_sec_pos);
       
        read1->lclip = 0; read1->rclip = read1->tru_sec_pos;
        read2->lclip = 0; read2->rclip = read2->tru_sec_pos;
       
        // Making a new sequence from these two overlapped:
       
        return true;
    } 
    
    return false;
}

void LoadAdapters(std::string filename, bool custom) {
    if (custom) {
        std::string str;
        std::fstream infile;
        /*Open given file:*/
        infile.open(filename.c_str(), std::fstream::in);
        
        //Loop thru all lines in input file:
        while ( std::getline(infile, str) ) {
            remove( str.begin(), str.end(), ' ' );
            if(str[0]== '>') 
            {
                continue;
            }
        
            //To uppercase:
            stoupper(str);
        
            //Getting the adapter: 
            adapters.push_back(str);
        }
    } else {
        adapters.push_back(tmpl_i5_1);
        adapters.push_back(tmpl_i7_1);
    }
}
