/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

//performs thinning operation on a binary image using Lam, L., Seong-Whan Lee, and Ching Y. Suen,
// "Thinning Methodologies-A Comprehensive Survey," IEEE Transactions on Pattern Analysis and Machine
// Intelligence, Vol 14, No. 9, September 1992, page 879, bottom of first column through top of second column.
//
// Coded by Asanka Wickramasinghe <asanka424@gmail.com>

#include "precomp.hpp"

void cv::thin(cv::Mat &input, cv::Mat &output, int itr)
{
    CV_Assert( input.type() == CV_8UC1 );
    output = input.clone();


    //scanning through the image
    uchar x1=0x00;
    uchar x2=0x00;
    uchar x3=0x00;
    uchar x4=0x00;
    uchar x5=0x00;
    uchar x6=0x00;
    uchar x7=0x00;
    uchar x8=0x00;

    bool changes_made = true;
    int runs = 0;


    // prepare two subfields in checkerboard pattern

    while(changes_made )
    {
        runs++;
        changes_made = false;
        std::vector<cv::Point> points_delete;
        for(int i = 0; i < output.rows; i++)
        {
            uchar *drow,*drow_above,*drow_below;
            //get the address of raw i
            drow = output.ptr<uchar>(i);
            if (i != 0)
                drow_above = output.ptr<uchar>(i-1);
            if(i != (output.rows -1) )
                drow_below = output.ptr<uchar>(i+1);
            for(int j = 0; j< output.cols; j++)
            {
                if (drow[j] != 0x00)
                {
                    x1 = (j == (output.cols - 1))? 0 : drow[j+1];
                    x3 = (i == 0)? 0 : drow_above[j];
                    x5 = (j == 0)? 0 : drow[j-1];
                    x7 = (i == (output.rows - 1))? 0 : drow_below[j];
                    x2 = ((i == 0) || (j == (output.cols - 1)))? 0 : drow_above[j+1];
                    x4 = ((i == 0) || (j == 0))? 0 :drow_above[j-1];
                    x6 = ((i == (output.rows - 1)) || (j == 0))? 0 : drow_below[j-1];
                    x8 = ((i == (output.rows - 1)) || (j == (output.cols - 1)))? 0 : drow_below[j+1];


                    //check for G1
                    int b1= 0 ,b2 = 0 ,b3 = 0 ,b4 = 0;
                    int sum = 0;
                    bool satisfy = false;
                    if((x1 == 0x00)&((x2==0xff) | (x3 == 0xff)))
                        b1 = 1;
                    if((x3 == 0x00)&((x4==0xff) | (x5 == 0xff)))
                        b2 = 1;
                    if((x5 == 0x00)&((x6==0xff) | (x7 == 0xff)))
                        b3 = 1;
                    if((x7 == 0x00)&((x8==0xff) | (x1 == 0xff)))
                        b4 = 1;
                    sum = b1 + b2 + b3 + b4;
                    //uchar notx = (x6 | x7 | ~x4);
                    if (sum == 1)// G1 satisfied go to check G2
                    {
                        int p1=0, p2=0, p3=0, p4=0 , q1=0, q2=0, q3=0, q4=0;
                        int sum1 = 0, sum2 = 0;
                        if((x1 | x2 )== 0xff)
                            p1 = 1;
                        if((x3 | x4 )== 0xff)
                            p2 = 1;
                        if((x5 | x6 )== 0xff)
                            p3 = 1;
                        if((x7 | x8 )== 0xff)
                            p4 = 1;
                        sum1 = p1 + p2 + p3+ p4;

                        if((x2 | x3 )== 0xff)
                            q1 = 1;
                        if((x4 | x5 )== 0xff)
                            q2 = 1;
                        if((x6 | x7 )== 0xff)
                            q3 = 1;
                        if((x8 | x1 )== 0xff)
                            q4 = 1;
                        sum2 = q1 + q2 + q3+ q4;
                        if( (std::min(sum1,sum2)>= 2) & (std::min(sum1,sum2)<=3))//g2 is also satisfied
                        {
                            if(((x2 | x3 | ~x8) & x1) == 0x00)
                                satisfy = true;
                            if(satisfy)
                            {
                                changes_made=true;
                                if( runs == itr)
                                    changes_made = false;

                                points_delete.push_back(cv::Point(j,i));
                            }
                        }
                        else //G2 not satisfied goto next pixel
                        {
                            continue;
                        }
                    }
                    else // G1 not satisfied go to the next pixel
                    {
                        continue;
                    }
                }
            }

        }

        for(int m =0; m<(int)points_delete.size(); m++)
            output.at<uchar>(points_delete[m].y,points_delete[m].x) = 0x00;
        //clear vector.
        points_delete.clear();
        // second iteration
        for(int i = 0; i < output.rows; i++)
        {
            uchar *drow,*drow_above,*drow_below;
            //get the address of raq i
            drow = output.ptr<uchar>(i);
            if (i != 0)
                drow_above = output.ptr<uchar>(i-1);
            if(i != (input.rows -1) )
                drow_below = output.ptr<uchar>(i+1);
            for(int j = 0; j< output.cols; j++)
            {
                if (drow[j] != 0x00)
                {

                    x1 = (j == (output.cols - 1))? 0 : drow[j+1];
                    x3 = (i == 0)? 0 : drow_above[j];
                    x5 = (j == 0)? 0 : drow[j-1];
                    x7 = (i == (output.rows - 1))? 0 : drow_below[j];
                    x2 = ((i == 0) || (j == (output.cols - 1)))? 0 : drow_above[j+1];
                    x4 = ((i == 0) || (j == 0))? 0 :drow_above[j-1];
                    x6 = ((i == (output.rows - 1)) || (j == 0))? 0 : drow_below[j-1];
                    x8 = ((i == (output.rows - 1)) || (j == (output.cols - 1)))? 0 : drow_below[j+1];

                    //check for G1
                    int b1= 0 ,b2 = 0 ,b3 = 0 ,b4 = 0;
                    int sum = 0;
                    bool satisfy = false;
                    if((x1 == 0x00)&((x2==0xff) | (x3 == 0xff)))
                        b1 = 1;
                    if((x3 == 0x00)&((x4==0xff) | (x5 == 0xff)))
                        b2 = 1;
                    if((x5 == 0x00)&((x6==0xff) | (x7 == 0xff)))
                        b3 = 1;
                    if((x7 == 0x00)&((x8==0xff) | (x1 == 0xff)))
                        b4 = 1;
                    sum = b1 + b2 + b3 + b4;

                    if (sum == 1)// G1 satisfied go to check G2
                    {
                        int p1=0, p2=0, p3=0, p4=0 , q1=0, q2=0, q3=0, q4=0;
                        int sum1 = 0, sum2 = 0;
                        if((x1 | x2 )== 0xff)
                            p1 = 1;
                        if((x3 | x4 )== 0xff)
                            p2 = 1;
                        if((x5 | x6 )== 0xff)
                            p3 = 1;
                        if((x7 | x8 )== 0xff)
                            p4 = 1;
                        sum1 = p1 + p2 + p3+ p4;

                        if((x2 | x3 )== 0xff)
                            q1 = 1;
                        if((x4 | x5 )== 0xff)
                            q2 = 1;
                        if((x6 | x7 )== 0xff)
                            q3 = 1;
                        if((x8 | x1 )== 0xff)
                            q4 = 1;
                        sum2 = q1 + q2 + q3+ q4;
                        if( (std::min(sum1,sum2)>= 2) & (std::min(sum1,sum2)<=3))//g2 is also satisfied
                        {
                            //check for checkerboard pattern
                            if(((x6 | x7 | ~x4) & x5) == 0x00)
                                satisfy = true;
                            if(satisfy)
                            {
                                changes_made=true;
                                if( runs == itr)
                                    changes_made = false;
                                points_delete.push_back(cv::Point(j,i));
                                //drow[j] = 0x00;
                            }
                        }
                        else //G2 not satisfied goto next pixel
                        {

                            continue;
                        }
                    }
                    else // G1 not satisfied go to the next pixel
                    {
                        continue;
                    }
                }
            }
        }
        for(int m =0; m<(int)points_delete.size(); m++)
            output.at<uchar>(points_delete[m].y,points_delete[m].x) = 0x00;

        points_delete.clear();

    }
    return;
}
