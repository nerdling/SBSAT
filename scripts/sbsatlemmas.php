<?PHP
$SYScolors = 1;
$SYSappname = "sbsatlemmas";
$appdir = "/home/mkouril/public_html/sbsat/";
$dir = "/home/weaversa/public_html/SBSAT_files/lemma_files/";
//$gnuplot = "/opt/gnu/bin/gnuplot";
//$gnuplot = "/usr/bin/gnuplot";
//$gnuplot = $appdir."gnuplot";
$gnuplot = "/home/mkouril/bin/gnuplot-4.2.0/bin/gnuplot";
$choose_file = "Please choose a file";
// $1  = c
// $2  = length
// $3  = moved up lpq
// $4  = conflicts
// $5  = age
// $6  = inferences
// $7  = referenced (0|1)
// $8  = age when first useful
// $9  = age when last useful
// $10 = lemma number
// $11 = lemma currently SAT (0|1)
// $12 = slid lemma
// $13 = lemma came from smurf
// $14 = num smurfs referenced
//---------------------
// histogram -- number of times the line occurs
$histogram = " | sort -g | uniq -c | awk 'BEGIN { N=0 } { N=N+1; while (N<$2) { print N,0; N=N+1; } print $2,$1 }' \" using 1:2 ";
// average -- average number $2 per number $1
$average = " | sort -g | awk 'BEGIN { AVG=0; N=0;COUNT=0;SUM=0 } { if (N != $1) { if (N!=0) print N, SUM/COUNT; N=N+1; while(N!=$1) { print N,0; N=N+1; }; SUM=0; COUNT=0; } SUM=SUM+$2; COUNT=COUNT+1;  }'\" using 1:2 ";
//------------------------
$desc = array();

// histograms
array_push($desc, array("--v-v-v-v-- Histograms --v-v-v-v--", "", ""));
array_push($desc, array("All Lemma length", "Lemma length",
        "awk '$0 ~ /^c/ { print $2 }' "));
array_push($desc, array("Lemma age/100", "Lemma age",
        "awk '$0 ~ /^c/ { print int($5/100); }' "));
array_push($desc, array("Useless Lemma age/100", "Lemma age",
        "awk '$0 ~ /^c/ && $3 ~ /^0$/ && $4 ~ /^0$/ && $6 ~ /^0$/ && $7 ~ /^0$/ { print int($5/100); }' "));
array_push($desc, array("Lemma moved up", "Move-ups",
        "awk '$0 ~ /^c/ && $3 > 0 { print $3; }' "));
array_push($desc, array("Num Inferences", "Num Inferences",
        "awk '$0 ~ /^c/ && $6 > 0 { print int($6/1); }' "));
array_push($desc, array("First Used Age Lemma", "Lemma age",
        "awk '$0 ~ /^c/ && $8 > 0 { print int($8/100); }' "));
array_push($desc, array("Last Used Age Lemma", "Lemma age",
        "awk '$0 ~ /^c/ && $9 > 0 { print int($9/100); }' "));
array_push($desc, array("How long ago Used Lemma", "Lemma intact time",
        "awk '$0 ~ /^c/ && $9 > 0 { print int(($5-$9)/100); }' "));
array_push($desc, array("Useless Lemma length", "Lemma length",
        "awk '$0 ~ /^c/ && $3 ~ /^0$/ && $4 ~ /^0$/ && $6 ~ /^0$/ && $7 ~ /^0$/  { print $2 }'  "));
array_push($desc, array("Useful Lemma length", "Lemma length",
        "awk '$0 ~ /^c/ && ( $3 !~ /^0$/ || $4 !~ /^0$/ || $6 !~ /^0$/ || $7 !~ /^0$/ )  { print $2 }' "));
array_push($desc, array("Moved Up Lemma length", "Lemma length",
        "awk '$0 ~ /^c/ && $3 > 0 { print $2 }'  "));
array_push($desc, array("Contradiction>0 Lemma length", "Lemma length",
        "awk '$0 ~ /^c/ && $4 > 0 { print $2 }'  "));
array_push($desc, array("Inferences>0 Lemma length", "Lemma length",
        "awk '$0 ~ /^c/ && $6 > 0 { print $2 }'  "));
array_push($desc, array("Slid Lemma length", "Slid Lemma length",
        "awk '$0 ~ /^c/ && $12 > 0 { print $2 }'  "));
array_push($desc, array("Num Slid Lemma inferences", "Num Slid Lemma inferences",
        "awk '$0 ~ /^c/ && $12 > 0 && $6 > 0 { print int($6/1); }'  "));
array_push($desc, array("Num Smurfs used to create lemma", "Num Smurfs used to create lemma",
        "awk '$0 ~ /^c/ && $14 > 0 { print int($14/1); }'  "));
array_push($desc, array("Num Smurfs used to create slid lemma", "Num Smurfs used to create slid lemma",
        "awk '$0 ~ /^c/ && $14 > 0 && $12 > 0 { print int($14/1); }'  "));
array_push($desc, array("Useful Slid Lemma length", "Lemma length",
        "awk '$0 ~ /^c/ && $12 > 0 && ( $3 !~ /^0$/ || $4 !~ /^0$/ || $6 !~ /^0$/ || $7 !~ /^0$/ )  { print $2 }' "));

// averages
array_push($desc, array("--v-v-v-v-- Averages --v-v-v-v--", "", ""));
array_push($desc, array("All Lemma length,move-ups", "Lemma length",
        "awk '$0 ~ /^c/ { print $2,$3; }' "));
array_push($desc, array("Useful Lemma length,move-ups", "Lemma length",
        "awk '$0 ~ /^c/ && $3 > 0 { print $2,$3; }' "));
array_push($desc, array("All Lemma length,conflicts", "Lemma length",
        "awk '$0 ~ /^c/ { print $2,$4; }' "));
array_push($desc, array("All Lemma length,age ", "Lemma length",
        "awk '$0 ~ /^c/ { print $2,$5; }' "));
array_push($desc, array("All Lemma length,inferences", "Lemma length",
        "awk '$0 ~ /^c/ { print $2,$6; }' "));
array_push($desc, array("Useless Lemma age, length", "Lemma age",
        "awk '$0 ~ /^c/ && $3 ~ /^0$/ && $4 ~ /^0$/ && $6 ~ /^0$/ && $7 ~ /^0$/ { print int($5/10),$2; }' "));
array_push($desc, array("Usefull Lemma length, life span/100", "Lemma length",
        "awk '$0 ~ /^c/ && $9 > 0 { print $2,int(($9-$8)/100); }' "));
array_push($desc, array("Usefull lemma length, last used age/100", "Lemma length",
        "awk '$0 ~ /^c/ && $9 > 0 { print $2,int(($9)/100); }' "));
array_push($desc, array("--v-v-v-v-- Custom --v-v-v-v--", "", ""));
array_push($desc, array("Custom Lemma length histogram 1", "Lemma length",
        "awk ' $0 ~ /^c/ && $8 > 3000 { print $2 }'  "));
array_push($desc, array("Custom Lemma length histogram 2", "Lemma length",
        "awk ' $0 ~ /^c/ && $8 > 5000 { print $2 }'  "));
array_push($desc, array("Custom Lemma length histogram 3", "Lemma length",
        "awk ' $0 ~ /^c/ && $8 > 7000 { print $2 }'  "));
array_push($desc, array("Inferences by lemma length", "Lemma length",
		  "awk '$0 ~ /^c/ { if ($6 != 0) {N=0; while(N<$6) { print $2; N=N+1; } } }'"));

$gtypes = array();
$gtypes[0] = array("Histogram", $histogram);
$gtypes[1] = array("Average", $average);

$allcolors = array();
array_push($allcolors, array("000000", "Black"));
array_push($allcolors, array("FFFFFF", "White"));
array_push($allcolors, array("FF0000", "Red"));
array_push($allcolors, array("00FF00", "Green"));
array_push($allcolors, array("0000FF", "Blue"));
array_push($allcolors, array("00FFFF", "Cyan"));
array_push($allcolors, array("FF00FF", "Magenta"));
array_push($allcolors, array("FFFF00", "Yellow"));

function color_picker($text, $j)
{
   global $SYScolors;
   global $colors;
   global $allcolors;
   if ($SYScolors == 0) return;

   if ($text != "") {
      echo ("$text&nbsp;");
   }
   echo "<select name=\"colors[$j]\">";
   for ($i=0; isset($allcolors[$i]); $i++)
         if ($allcolors[$i][0] == $colors[$j])
            echo "<option value=".$allcolors[$i][0]." selected>".$allcolors[$i][1]."</option>\n";
         else
            echo "<option value=".$allcolors[$i][0].">".$allcolors[$i][1]."</option>\n";
   echo "</select>"; 
}

$exist_files = array();
array_push($exist_files, $choose_file);

// Open a known directory, and proceed to read its contents
if (is_dir($dir)) {
   if ($dh = opendir($dir)) {
      while (($file = readdir($dh)) !== false) {
         if ($file != '.' && $file != '..')
            array_push($exist_files, $file);
         //print "filename: $file : filetype: " . filetype($dir . $file) . "\n";
      }
      closedir($dh);
   }
}

$termtypes = array();
/* png color : background, border, x, y, plotting.... */
/* example png small color xffffff xffffff x000000 x000000 xff0000 x00ff00 .. */
array_push($termtypes, array("PNG Default Colors","image/png", "png color", 0, "png"));
if ($SYScolors)
array_push($termtypes, array("PNG Custom Colors","image/png", "png small color", 1, "png"));
array_push($termtypes, array("PNG Mono","image/png", "png small mono", 0, "png"));
array_push($termtypes, array("PS Color","image/eps", "postscript eps color", 0, "eps"));
array_push($termtypes, array("PS Mono","image/eps", "postscript eps mono", 0, "eps"));

$logscales = array();
$logscales[0] = "";
$logscales[1] = "x";
$logscales[2] = "y";
$logscales[3] = "xy";

$logscale = $_POST["logscale"];
if (!isset($logscale)) $logscale = $_GET["logscale"];

$show = $_POST["show"];
if (!isset($show)) $show = $_GET["show"];

$gtype = $_POST["gtype"];
if (!isset($gtype)) $gtype = $_GET["gtype"];

$files = $_POST["files"];
if (!isset($files)) $files = $_GET["files"];

$enabled = $_POST["enabled"];
if (!isset($enabled)) $enabled = $_GET["enabled"];

$debug = $_POST["debug"];
if (!isset($debug)) $debug = $_GET["debug"];

$debug_src = $_POST["debug_src"];
if (!isset($debug_src)) $debug_src = $_GET["debug_src"];

$termtype = $_POST["termtype"];
if (!isset($termtype)) $termtype = $_GET["termtype"];

$lines = $_POST["lines"];
if (!isset($lines)) $lines = $_GET["lines"];

$filedesc = $_POST["filedesc"];
if (!isset($filedesc)) $filedesc = $_GET["filedesc"];

$title = $_POST["title"];
if (!isset($title)) $title = $_GET["title"];

$colors = $_POST["colors"];
if (!isset($colors)) $colors = $_GET["colors"];

if (!isset($x_axis)) $x_axis = 35;
if (!isset($termtype)) $termtype = 0;
if (!isset($lines)) $lines = 3;
if (!isset($title)) $title = "SBSAT";
if (!isset($colors)) {
   $colors[0] = 'FFFFFF';
   $colors[1] = '000000';
   $colors[2] = '000000';
   $colors[3] = 'FF0000';
}

if (!isset($colors[3])) $colors[3] = 'FF0000';
if (!isset($colors[4])) $colors[4] = '00FF00';
if (!isset($colors[5])) $colors[5] = '0000FF';
if (!isset($colors[6])) $colors[6] = '00FFFF';
if (!isset($colors[7])) $colors[7] = 'FF00FF';
if (!isset($colors[8])) $colors[8] = 'FFFF00';


if (isset($filedesc) && $filedesc!="" && $filedesc[0]!=".") {
   echo "<HTML><BODY>\n";
   echo "File Description for $filedesc<br>\n";
   $fp = fopen($dir.$filedesc, "r");
   if ($fp) {
      while (($line = fgets($fp, 2048)) &&
             ($line[0] == '#')) {
         echo $line;
      }
      fclose($fp);
   }
   echo "</BODY></HTML>\n";
   exit;
}
if (!isset($_GET["button"]) && !isset($_POST["button"]) &&
    !isset($_GET["download"]) && !isset($_POST["download"])) {
   /* draw html form */
   if (isset($_GET["addline"]) || isset($_POST["addline"]))
       $lines++;
   if (isset($_GET["removeline"]) || isset($_POST["removeline"]))
      if ($lines>1) $lines--;
   echo "<HTML><BODY><FORM NAME=myform METHOD=POST ACTION=\"$SYSappname.php\">\n";
   echo "<B>SBSAT Lemmas</B> -- Plots the lemma histogram generated <BR>\n";
   echo "<B>SBSAT Lemmas</B> -- by sbsat option --reports 1 (and key 'L')<BR>\n";
   echo "Output Format: &nbsp;";
   echo "<select name=termtype>";
   for ($i=0; $i < 40; $i++)
      if (isset($termtypes[$i]))
         if ($termtype == $i)
            echo "<option value=$i selected>".$termtypes[$i][0]."</option>\n";
         else
            echo "<option value=$i>".$termtypes[$i][0]."</option>\n";
   echo "</select>"; 
   color_picker("BG:", 0); // background
   color_picker("Text:", 1); // fore??
   color_picker("?:", 2); // axes
   echo "<HR>";
   echo "<input type=hidden NAME=lines value=$lines>\n";
   echo "<input type=hidden NAME=filedesc value=''>\n";
   echo "Graph Title:&nbsp;";
   echo "<input type=text NAME=title size=80 value='$title'>\n";

   echo "Logscale: &nbsp;";
   echo "<select name=logscale>";
   for ($i=0; $i < 40; $i++) 
   {
      if (isset($logscales[$i])) {
         if ($logscales[$i] == $logscale)
            echo "<option value=\"".$logscales[$i]."\" selected>".$logscales[$i]."</option>\n";
         else
            echo "<option value=\"".$logscales[$i]."\">".$logscales[$i]."</option>\n";
      }
   }
   echo "</select>"; 
   echo "<br>\n";


   for ($j=0;$j<$lines;$j++) {
      if ($enabled[$j])
         echo "<input type=checkbox name=\"enabled[$j]\" checked>";
      else
         echo "<input type=checkbox name=\"enabled[$j]\">";
      echo "File: &nbsp;";
      echo "<select name=\"files[$j]\">\n";
      for ($i=0;isset($exist_files[$i]);$i++)
         if ($files[$j] == $exist_files[$i])
            echo "<option value='".$exist_files[$i]."' selected>".$exist_files[$i]."</option>";
         else
            echo "<option value='".$exist_files[$i]."'>".$exist_files[$i]."</option>";
      echo "</select>\n";
      //echo "<input type=submit name=filedescbut onClick=\"this.form.target='_blank'; var d=new Date(); document.myform.action='$SYSappname.php?'+d.getTime()+'&filedesc='+document.myform.elements['files[$j]'].value;\" value=\"?\">\n";
      //echo "<input type=submit name=filedescbut onClick=\"this.form.target='_blank'; this.form.filedesc.value=this.form.elements['files[$j]'].value;\" value=\"?\">\n";
      //echo "<input type=submit name=filedescbut onClick=\"this.form.target='_blank'; this.form.filedesc.value=document.myform.elements['files[$j]'].value;\" value=\"?\">\n";
      echo "<input type=submit name=filedescbut onClick=\"var lb=document.myform.elements['files[$j]']; this.form.filedesc.value=lb.options[lb.selectedIndex].value; this.form.target='_blank';\" value=\"?\">\n";
      //echo "Show: &nbsp;";
      echo "<select name=\"show[]\">";
      for ($i=0; $i < 40; $i++)
         if (isset($desc[$i])) {
            if ($show[$j] == $i)
               echo "<option value=$i selected>".$desc[$i][0]."</option>\n";
            else
               echo "<option value=$i>".$desc[$i][0]."</option>\n";
         }
      echo "</select>\n"; 
      echo "<select name=\"gtype[]\">";
      for ($i=0; $i < 40; $i++)
         if (isset($gtypes[$i])) {
            if ($gtype[$j] == $i)
               echo "<option value=$i selected>".$gtypes[$i][0]."</option>\n";
            else
               echo "<option value=$i>".$gtypes[$i][0]."</option>\n";
         }
      echo "</select>\n"; 
      color_picker("",3+$j); 
      echo "<br>\n"; 
   }

   echo "Debug: &nbsp;";
   //if ($debug) echo "<input type=checkbox name=debug checked> &nbsp;"; else
   echo "<input type=checkbox name=debug> &nbsp;";
   //if ($debug_src) echo "<input type=checkbox name=debug_src checked> &nbsp;"; else
   echo "<input type=checkbox name=debug_src> &nbsp;";
   echo "<input type=submit name=button onClick=\"this.form.target='_blank';this.form.filedesc.value='';var d=new Date(); document.myform.action='$SYSappname.php?'+d.getTime();\" value=\"Generate Graph\">\n";
   echo "<input type=submit name=download onClick=\"this.form.target='_blank';this.form.filedesc.value='';var d=new Date(); document.myform.action='$SYSappname.php?'+d.getTime();\" value=\"Download\">\n";
   echo "<input type=submit name=addline onClick=\"this.form.target='_self';this.form.filedesc.value='';\" value=\"Add Line\">\n";
   echo "<input type=submit name=removeline onClick=\"this.form.target='_self';this.form.filedesc.value='';\" value=\"Remove Line\">\n";
   echo "<input type=submit name=method onClick=\"this.form.target='_self';this.form.method='get';this.form.filedesc.value='';\" value=\"Get Full URL(to save)\">\n";
   //echo "</FORM><HR><FORM METHOD=GET ACTION=$SYSappname.php TARGET=_blank>";
   //echo "<input type=submit name=filedesc value=\"Get File Description\">\n";
   echo "</FORM></BODY></HTML>";
   exit;
}
/*
$filename = "dlx2_cc.csv";
$x_axis = 35;
$show = array();
$files = array();
$enabled = array();
array_push($show, 22); array_push($files, $filename); array_push($enabled, 1);
array_push($show, 24); array_push($files, $filename); array_push($enabled, 1);
array_push($show, 26); array_push($files, $filename); array_push($enabled, 1);
*/

$error = 0;
$termstring = $termtypes[$termtype][2];
if ($termtypes[$termtype][3]) {
   for ($i=0; isset($colors[$i]); $i++)
      if ($i<3 || isset($enabled[$i-3]))
         $termstring .= " x".$colors[$i];
};
$input = "
set term $termstring;
#set term png color; 
set title '$title';
#set size 0.5 0.5; #640x480-> 320x240
#set xrange [0:];
#set yrange [0:];
";

if ($logscale != "")
$input .= "set logscale $logscale;
";

  for ($index = 0; $index<$lines; $index++) {
     if (isset($enabled[$index])) {
        $_show = $show[$index];
        $_gtype = $gtype[$index];
        $file = $files[$index];
        if ($file == $choose_file) {
           $error = 1;
        }
        if (!isset($line)) {
           $line = "plot ";
           $xlabel = "set xlabel '".$desc[$_show][1]."';
           ";
        } else {
           $line .= ", ";
        }
        $line .= "\"< cat ".$dir.$file." | ".$desc[$_show][2].$gtypes[$_gtype][1];
        $line .= " title '".$desc[$_show][0]." ".$gtypes[$_gtype][0]." in $file' with steps ";
     }
  }
  $line .= ";";
  $input .= $xlabel;
  $input .= $line;

if ($debug_src)
    $errredir = " 2>&1 ";
else
    $errredir = " 2> /dev/null ";

header("Pragma: no-cache");
header("Pragma: public");
header("Expires: 0"); // set expiration time
header("Cache-Control: must-revalidate, post-check=0, pre-check=0"); 

$Cmd = $gnuplot.$errredir." << END\n" .$input."\nEND\n";
if ($debug || $error) {
   header("Content-type: text/plain");
   echo $Cmd;
   if (!$debug_src) exit;
} else 
if (isset($_GET["download"]) || isset($_POST["download"])) {
  header("Content-type: application/binary");
  header("Content-Disposition: attachment; filename=".$SYSappname.".".$termtypes[$termtype][4].";");
}
else {
  header("Content-type: ".$termtypes[$termtype][1]);
}

$Cmd = ereg_replace('\$', '\\$', $Cmd);
$fp = popen($Cmd, 'r');
fpassthru($fp);
//pclose($fp);

