<?PHP
$SYSappname = "sbsatplot";
$SYScolors = 0;
$appdir = "/home/mkouril/public_html/sbsat/";
$dir = "/home/mkouril/public_html/sbsat/csv/";
//$gnuplot = "/opt/gnu/bin/gnuplot";
//$gnuplot = "/usr/bin/gnuplot";
$gnuplot = $appdir."gnuplot";
$choose_file = "Please choose a file";

$desc = array();
$desc[0] = "Non bounded inferences"; // NO_ERROR
$desc[1] = "Backtracks by smurfs"; // ERR_BT_SMURF 1
$desc[2] = "Backtracks by ANDs"; // ERR_BT_SPEC_FN_AND 2
$desc[3] = "Backtracks by XORs"; // ERR_BT_SPEC_FN_XOR 3
$desc[4] = "Backtracks by lemmas"; // ERR_BT_LEMMA 4
$desc[5] = "Solutions"; // ERR_BT_SOL_LEMMA 5
//$desc[6] = "Limits violation"; // ERR_LIMITS 6

$desc[10] = "Inferences by smurfs"; // INF_SMURF        10
$desc[11] = "Inferences by ANDs"; // INF_SPEC_FN_AND  11 
$desc[12] = "Inferences by XORs"; // INF_SPEC_FN_XOR  12 
$desc[13] = "Inferences by Lemmas"; // INF_LEMMA        13
//$desc[15] = "Smurf States"; // SMURF_STATES     15
//$desc[16] = "Max Num Solutions"; // NUM_SOLUTIONS    16
//$desc[17] = "BDD Node Find"; // BDD_NODE_FIND    17
//$desc[18] = "BDD Node New"; // BDD_NODE_NEW     18
//$desc[19] = "BDD Node Steps"; // BDD_NODE_STEPS   19
//$desc[20] = "Smurf Node Find"; // SMURF_NODE_FIND    20
//$desc[21] = "Smurf Node New"; // SMURF_NODE_NEW     21
$desc[22] = "Num Backtracks"; // NUM_BACKTRACKS   22
$desc[23] = "Num Backjump Jumps"; // NUM_BACKJUMPS    23
$desc[24] = "Num Backjumped Choicepoints"; // NUM_TOTAL_BACKJUMPS      24
$desc[25] = "Num Autarky Jumps"; // NUM_AUTARKIES    25
$desc[26] = "Num Autarked Choicepoints"; // NUM_TOTAL_AUTARKIES      26
$desc[27] = "Num Choice Points"; #define NUM_CHOICE_POINTS     27
$desc[28] = "Num Dep Choice Points"; #define HEU_DEP_VAR      28

// MAX_COUNTER      30
//$desc[31] = "Total running time"; // RUNNING_TIME      1
//$desc[32] = "Smurf Build time"; // BUILD_SMURFS      2
//$desc[33] = "Preproc time"; // PREPROC_TIME      3
//$desc[34] = "Brancher time"; // BRANCHER_TIME     4
$desc[35] = "Current time"; // CURRENT_TIME      5
$desc[36] = "Current Brancher Progress"; // PROGRESS          6 
// MAX_COUNTER_F    10

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

$x_axis = $_POST["x_axis"];
if (!isset($x_axis)) $x_axis = $_GET["x_axis"];

$show = $_POST["show"];
if (!isset($show)) $show = $_GET["show"];

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
   echo "<B>SBSAT Plot</B> -- Plots the traces generated by sbsat option --csv-trace-file filename<BR>\n";
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
   echo "<input type=text NAME=title size=80 value='$title'><BR>\n";

   echo "X Axis: &nbsp;";
   echo "<select name=x_axis>";
   for ($i=0; $i < 40; $i++) 
   {
      if (isset($desc[$i])) {
         if ($i == $x_axis)
            echo "<option value=$i selected>".$desc[$i]."</option>\n";
         else
            echo "<option value=$i>".$desc[$i]."</option>\n";

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
      echo "Show: &nbsp;";
      echo "<select name=\"show[]\">";
      for ($i=0; $i < 40; $i++)
         if (isset($desc[$i])) {
            if ($show[$j] == $i)
               echo "<option value=$i selected>".$desc[$i]."</option>\n";
            else
               echo "<option value=$i>".$desc[$i]."</option>\n";

            if ($show[$j] == $i+100)
               echo "<option value=".($i+100)." selected>".$desc[$i]."(per Second)</option>\n";
            else
               echo "<option value=".($i+100).">".$desc[$i]."(per Second)</option>\n";
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
set xlabel '".$desc[$x_axis]."';
";

  $x_axis++;
  for ($index = 0; $index<$lines; $index++) {
     if (isset($enabled[$index])) {
        $item = $show[$index];
        $file = $files[$index];
        if ($file == $choose_file) {
           $error = 1;
        }
        if (!isset($line)) {
           $line = "plot ";
        } else {
           $line .= ", ";
        }
        if ($item < 100) {
           $line .= "'".$dir.$file."' using $x_axis:".($item+1)." title '".$desc[$item]." in $file' with lines ";
        } else {
           $item -= 100;
            $line .= "\"< $appdir/awk 'BEGIN { N=0; Y=0; } \$0 !~ /^#/ { C=\\\$".($item+1)."; S=\\\$36;X=\\\$".$x_axis."; D=S-Y; if (D==0) D=1; print X,1.0*(C-N)/(D); N=C; Y=S; }' ".$dir.$file."\" using 1:2 title '".$desc[$item]." per Second in $file' with lines ";
           // awk 'BEGIN { N=0 } $0 !~ /^#/ { C=$2; print C-N; N=C }' csv/dlx2_cc.0.csv
        }
     }
  }
  $line .= ";";
  $input .= $line;

if ($debug_src)
    $errredir = " 2>&1 ";
else
    $errredir = " 2> /dev/null ";

header("Pragma: no-cache");
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
} else     
   header("Content-type: ".$termtypes[$termtype][1]);

$fp = popen($Cmd, 'r');
fpassthru($fp);
//pclose($fp);

