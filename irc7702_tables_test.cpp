// 7PP and CVAT corridor from first principles--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "irc7702_tables.hpp"

#include "et_vector.hpp"
#include "materially_equal.hpp"
#include "math_functions.hpp"
#include "ssize_lmi.hpp"
#include "test_tools.hpp"

#include <cfloat>                       // DBL_EPSILON
#include <cmath>                        // log()
#include <vector>

// These two arrays are pasted from the "corridor mult" and "7Pt"
// columns of 'validate_2001cso', after loading that spreadsheet
// in 'gnumeric' and formatting those columns to twenty decimals.
// They use UL commutation functions. See:
//   https://lists.nongnu.org/archive/html/lmi/2015-07/msg00004.html

static double const ss_ul_corr[100] =
{16.05428685334679173025
,15.58833599045649087600
,15.08131070200895074152
,14.56342500447235721595
,14.04800434985640045227
,13.54397833171609022429
,13.05507387234714933300
,12.58409661481673680328
,12.13030984655223143420
,11.69174091577074925397
,11.26914166185736476677
,10.86078105527353621085
,10.47029884268147448267
,10.09843359081480329564
, 9.74294658529263379876
, 9.40759852219825454256
, 9.09057733647785148889
, 8.79027993284445763322
, 8.50397918973384925323
, 8.22818279387080941945
, 7.96118932260834633041
, 7.70160265863622406357
, 7.44976553796342688685
, 7.20499258154732391546
, 6.96714059909527883008
, 6.73646229302490073820
, 6.51311660413237181189
, 6.29752334665072410758
, 6.08868484174302349032
, 5.88548788700455993705
, 5.68759233818214315903
, 5.49522616091999260846
, 5.30831030469870679411
, 5.12719475896684162564
, 4.95191511278486373726
, 4.78246091867951061971
, 4.61878315488500756913
, 4.46095911393538724354
, 4.30885324846834905088
, 4.16231281496283944676
, 4.02117206738939625410
, 3.88525584688570946312
, 3.75459497560775545821
, 3.62916166404261142375
, 3.50888104609130779821
, 3.39355762372614266908
, 3.28298850929638108553
, 3.17689558846831987537
, 3.07468880549077061559
, 2.97599858560200258850
, 2.88084470780052326333
, 2.78925893311157580001
, 2.70137019361723274002
, 2.61714709838817194409
, 2.53660248632115825629
, 2.45973786190931420492
, 2.38640466067929235550
, 2.31641149864538897774
, 2.24933351623363186889
, 2.18485859704871998588
, 2.12298548776600348731
, 2.06377060631972986116
, 2.00732734016649461140
, 1.95364688011445042370
, 1.90257560558083449465
, 1.85390645085427285821
, 1.80741123662186708998
, 1.76283474569483433747
, 1.72002394995400198496
, 1.67882810972211782286
, 1.63921097550600691761
, 1.60120703518178442160
, 1.56497463008854764688
, 1.53056280828563395779
, 1.49778239342608299189
, 1.46651641824560186045
, 1.43669325356442145747
, 1.40828569180480034362
, 1.38135623671030072224
, 1.35595612880361837682
, 1.33205157176719568568
, 1.30966078753973991233
, 1.28874851143154445232
, 1.26913015525378947146
, 1.25071057910177563244
, 1.23349050718607222699
, 1.21741784864651991782
, 1.20250900968492024923
, 1.18879112389207719502
, 1.17616396084234375863
, 1.16443752080775975166
, 1.15305360309844973088
, 1.14154811660101529469
, 1.12990850456893321407
, 1.11805918549405669538
, 1.10585007755591901990
, 1.09272887195287826678
, 1.07768889207696427768
, 1.05889467902677236744
, 1.03392745267212093907
};

static double const ss_ul_7pp[100] =
{0.00999062440751989980
,0.01028589716601673552
,0.01062994889232243140
,0.01100709650196978612
,0.01141070148832002557
,0.01183541561717901119
,0.01227894262614234906
,0.01273890281958302631
,0.01321604446917592178
,0.01371289447070540846
,0.01422886804024923145
,0.01476663382430560070
,0.01532082243677517636
,0.01588896142650659399
,0.01647329371041960158
,0.01706468830702987174
,0.01766329128405109278
,0.01826920380439612038
,0.01888577215846250559
,0.01951980733786566266
,0.02017525559338708904
,0.02085633674231307522
,0.02156255487066226217
,0.02229655784475696040
,0.02305946770995651388
,0.02385084039099466188
,0.02467017211649025557
,0.02551531634397709650
,0.02639053264687007266
,0.02730200324593424696
,0.02825306412357134547
,0.02924406198100011275
,0.03027703373115286228
,0.03135091541809587046
,0.03246623554060621558
,0.03362359115588391123
,0.03482374166899589174
,0.03606599422970604291
,0.03735138265340059993
,0.03868112554395738284
,0.04005664955242391884
,0.04147956289034327587
,0.04294811241142196950
,0.04446025296529201171
,0.04601393073756083335
,0.04760879558840754727
,0.04924490987244876244
,0.05092435295618532448
,0.05265819337044034404
,0.05445592655778288410
,0.05631857051817811682
,0.05824605305494993251
,0.06023316553704032711
,0.06227739915724234826
,0.06437337655335158282
,0.06651452375385756566
,0.06870008885031553270
,0.07093229090910557755
,0.07322803247632281298
,0.07560364239280463472
,0.07805934610230855220
,0.08058864677483222605
,0.08317610640496489460
,0.08580967539473746686
,0.08848738367896104295
,0.09121467879909027354
,0.09400583834207149048
,0.09688672842792392381
,0.09987797601076543741
,0.10300586867342216568
,0.10628627958701071521
,0.10972561873411729172
,0.11330637125813265709
,0.11702343505126171219
,0.12092139119378841894
,0.12504454546357654521
,0.12943750862444663996
,0.13413592524308720622
,0.13914635982676987203
,0.14447036818012279347
,0.15013201659901123652
,0.15613245573059561511
,0.16249080594919776410
,0.16930561735046611993
,0.17664266810670117835
,0.18447808708057295002
,0.19278062434737353037
,0.20145103153300336629
,0.21034011597622215284
,0.21938905920499771063
,0.22862636208321829301
,0.23858323922962185182
,0.24981209741923118828
,0.26220186143911550225
,0.28642313285880155460
,0.31717670783614265728
,0.35965728769722676850
,0.42686340219691948361
,0.56031960772985123231
,0.96718584791956385693
};

static double const ss_ol_corr[100] =
{16.03069515721970
,15.56545542916450
,15.05919070490880
,14.54207568908480
,14.02741865395940
,13.52413778894550
,13.03595534928800
,12.56567351181010
,12.11255664606990
,11.67463475724690
,11.25265884919660
,10.84490022682110
,10.45499411648920
,10.08367827587890
, 9.72871697105366
, 9.39386625032253
, 9.07731649360929
, 8.77746677844702
, 8.49159356632208
, 8.21620906614680
, 7.94961408895666
, 7.69041434829488
, 7.43895222208455
, 7.19454325862477
, 6.95704451334878
, 6.72670844298237
, 6.50369386229784
, 6.28842014111261
, 6.07989090207194
, 5.87699437244831
, 5.67939087980040
, 5.48730816704992
, 5.30066733657544
, 5.11981801243330
, 4.94479580197072
, 4.77559033227269
, 4.61215270269525
, 4.45456017660894
, 4.30267743185537
, 4.15635196932867
, 4.01541830136802
, 3.87970153695790
, 3.74923252419573
, 3.62398356964482
, 3.50387995898405
, 3.38872648535844
, 3.27832055910429
, 3.17238444761715
, 3.07032882594890
, 2.97178458990226
, 2.87677154683430
, 2.78532147558464
, 2.69756322259474
, 2.61346549280934
, 2.53304117542039
, 2.45629184138028
, 2.38306915684781
, 2.31318201343135
, 2.24620607000211
, 2.18182960005580
, 2.12005141288773
, 2.06092793502465
, 2.00457251268315
, 1.95097642680359
, 1.89998629486328
, 1.85139534102519
, 1.80497569334546
, 1.76047246308625
, 1.71773284834008
, 1.67660633172185
, 1.63705678130120
, 1.59911874599313
, 1.56295053110245
, 1.52860126016288
, 1.49588204901423
, 1.46467616317511
, 1.43491217369859
, 1.40656304375306
, 1.37969137924250
, 1.35434853909332
, 1.33050092572148
, 1.30816690878090
, 1.28731141765329
, 1.26775022859716
, 1.24938847835256
, 1.23222703314812
, 1.21621399207302
, 1.20136578942719
, 1.18770946309554
, 1.17514487873005
, 1.16348237607749
, 1.15216547131899
, 1.14073189550427
, 1.12916922360185
, 1.11740178851122
, 1.10527880649853
, 1.09224812150223
, 1.07730439700387
, 1.05861825755067
, 1.03377932830447
};

static double const ss_ol_7pp[100] =
{0.010005327186958
,0.010301016999909
,0.010645562907928
,0.011023256091546
,0.011427447066149
,0.011852778725528
,0.012296950914868
,0.012757579901923
,0.013235415035928
,0.013732987171806
,0.014249710382439
,0.014788257479081
,0.015343250090949
,0.015912211536579
,0.016497388214910
,0.017089634046416
,0.017689095179996
,0.018295872789306
,0.018913318467641
,0.019548254138137
,0.020204632276942
,0.020886679342528
,0.021593898359639
,0.022328941267110
,0.023092931684899
,0.023885424544973
,0.024705914983204
,0.025552252675176
,0.026428703850223
,0.027341460483306
,0.028293863627333
,0.029286260140180
,0.030320689819902
,0.031396086507395
,0.032512979072804
,0.033671965026394
,0.034873804442084
,0.036117802719780
,0.037404994687573
,0.038736600205397
,0.040114047455165
,0.041538946413990
,0.043009540227530
,0.044523779574169
,0.046079606410946
,0.047676669074214
,0.049315028941757
,0.050996767549288
,0.052732969936346
,0.054533145156035
,0.056398310810159
,0.058328392333208
,0.060318170371727
,0.062365129728319
,0.064463881836038
,0.066607839381289
,0.068796246114121
,0.071031321069015
,0.073329989610741
,0.075708601646018
,0.078167377428823
,0.080699804001149
,0.083290412982767
,0.085927129777381
,0.088607974722984
,0.091338396338728
,0.094132685084862
,0.097016735479080
,0.100011192647016
,0.103142368320708
,0.106426141130087
,0.109868909420875
,0.113453108667038
,0.117173603120152
,0.121075007777516
,0.125201654498742
,0.129598172492744
,0.134300204399513
,0.139314266113466
,0.144641852159782
,0.150306989508367
,0.156310753283924
,0.162672203024568
,0.169489903915145
,0.176829591075732
,0.184667243191684
,0.192971446212092
,0.201642732430611
,0.210531675161746
,0.219579312738766
,0.228814049726310
,0.238767148018836
,0.249990843980925
,0.262373528220005
,0.286591643152315
,0.317340642829014
,0.359815590009435
,0.427015751779345
,0.560465915779944
,0.967324430485689
};

/// Test CVAT corridor and seven-pay premium.

void Test_Corridor_and_7PP()
{
    double constexpr iglp = 0.04;
    std::vector<double> const naar_discount
        (100
        ,i_upper_12_over_12_from_i<double>()(iglp)
        );
    irc7702_tables z
        (mce_2001cso
        ,oe_orthodox
        ,oe_age_last_birthday
        ,mce_unisex
        ,mce_unismoke
        ,naar_discount
        ,1.0 / 12.0
        ,0
        ,100
        );
    std::vector<double> const& ul_corr = z.ul_corr();
    std::vector<double> const& ul_7pp  = z.ul_7pp ();
    std::vector<double> const& ol_corr = z.ol_corr();
    std::vector<double> const& ol_7pp  = z.ol_7pp ();

    // In the last year, the OL formula reduces to:
    //   NSP[omega-1] = vq * (i/delta) + vp
    //   =     (0.30285  / (1.0 + iglp)) * (iglp / ln((1.0 + iglp)))
    //   + (1 - 0.30285) / (1.0 + iglp)
    BOOST_TEST(materially_equal(0.30285, z.q_[99], 0.0));
    double ol_nsp99 =
          (       z.q_[99]  / (1.0 + iglp)) * (iglp / std::log((1.0 + iglp)))
        +  (1.0 - z.q_[99]) / (1.0 + iglp)
        ;
    BOOST_TEST(materially_equal(ol_corr[99], 1.0 / ol_nsp99, DBL_EPSILON));

    for(int j = 0; j < lmi::ssize(ol_corr); ++j)
        {
        // Values calculated here must match spreadsheet values to
        // within a minuscule tolerance.
        BOOST_TEST(materially_equal(ss_ul_corr[j], ul_corr[j]));
        BOOST_TEST(materially_equal(ss_ul_7pp [j], ul_7pp [j]));
        BOOST_TEST(materially_equal(ss_ol_corr[j], ol_corr[j]));
        BOOST_TEST(materially_equal(ss_ol_7pp [j], ol_7pp [j]));
        // The 0.0015 tolerance was found experimentally, not by any
        // sort of mathematical law. It represents the inherent
        // discrepancy between the UL method on the one hand, and the
        // OL method (with the i/delta approximation) on the other.
        BOOST_TEST(materially_equal(   ol_corr[j], ul_corr[j], 0.0015));
        BOOST_TEST(materially_equal(   ol_7pp [j], ul_7pp [j], 0.0015));
        }

    // At least for now, display plenty of detail.

    std::cout << std::endl;

    std::cout
        << ol_corr[99] << " OL corr[omega-1]\n"
        << 1.0 / ol_nsp99 << " vq * (i/delta) + vp\n"
        << std::endl
        ;

    std::cout << "q, UL corr, UL 7pp, OL corr, OL 7pp\n";
    for(int j = 0; j < lmi::ssize(ol_corr); ++j)
        {
        std::cout
            << j
            << '\t' << z.q_[j]
            << '\t' << ul_corr[j]
            << '\t' << ul_7pp[j]
            << '\t' << ol_corr[j]
            << '\t' << ol_7pp[j]
            << '\n';
        }
    std::cout << std::endl;

    std::vector<double> q12(lmi::ssize(ol_corr));
    assign(q12, apply_binary(coi_rate_from_q<double>(), z.q_, 1.0 / 12.0));
    std::cout << "q, q upper (12), UL corr, UL 7pp\n";
    for(int j = 0; j < lmi::ssize(ul_corr); ++j)
        {
        std::cout
            << j
            << '\t' << z.q_[j]
            << '\t' << q12[j]
            << '\t' << ul_corr[j]
            << '\t' << ul_7pp[j]
            << '\n';
        }
    std::cout << std::endl;

    std::cout << "UL corr, OL corr, relative error\n";
    for(int j = 0; j < lmi::ssize(ul_corr); ++j)
        {
        std::cout
            << j
            << '\t' << ul_corr[j]
            << '\t' << ol_corr[j]
            << '\t' << (ol_corr[j] - ul_corr[j]) / ul_corr[j]
            << '\n';
        }
    std::cout << std::endl;

    std::cout << "UL 7pp, OL 7pp, relative error\n";
    for(int j = 0; j < lmi::ssize(ul_corr); ++j)
        {
        std::cout
            << j
            << '\t' << ul_7pp[j]
            << '\t' << ol_7pp[j]
            << '\t' << (ol_7pp[j] - ul_7pp[j]) / ul_7pp[j]
            << '\n';
        }
    std::cout << std::endl;

    std::cout << "UL corr, 7pp: {calculated, spreadsheet, relative error}\n";
    for(int j = 0; j < lmi::ssize(ss_ul_corr); ++j)
        {
        std::cout
            << j
            << '\t' << ul_corr[j]
            << '\t' << ss_ul_corr[j]
            << '\t' << (ss_ul_corr[j] - ul_corr[j]) / ul_corr[j]
            << '\t' << ul_7pp[j]
            << '\t' << ss_ul_7pp[j]
            << '\t' << (ss_ul_7pp[j] - ul_7pp[j]) / ul_7pp[j]
            << '\n';
        }
    std::cout << std::endl;

    std::cout << "OL corr, 7pp: {calculated, spreadsheet, relative error}\n";
    for(int j = 0; j < lmi::ssize(ss_ul_corr); ++j)
        {
        std::cout
            << j
            << '\t' << ol_corr[j]
            << '\t' << ss_ol_corr[j]
            << '\t' << (ss_ol_corr[j] - ol_corr[j]) / ol_corr[j]
            << '\t' << ol_7pp[j]
            << '\t' << ss_ol_7pp[j]
            << '\t' << (ss_ol_7pp[j] - ol_7pp[j]) / ol_7pp[j]
            << '\n';
        }
}

int test_main(int, char*[])
{
    Test_Corridor_and_7PP();

    return EXIT_SUCCESS;
}
