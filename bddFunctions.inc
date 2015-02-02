<?php
//PIERRE
// task: to update bddTable with form2 answer
function addForm2Answer($form,$ip){
    $dentist=getPost('dentist');
    //toto
    $kine=getPost('kine');
    $ophtalmo=getPost('ophtalmo');
    $haircut=getPost('haircut');
    $esthetic=getPost('esthetic');
    $none=getPost('none');
    $other=getPost('other');
    $interest=getPost('interest');
    $price=getPost('price');
    $email=getPost('email');
    $comments=getPost('comments');
    $comments=addslashes($comments);
    $noError=update_Table_Condition_SetFieldsAndValues('stats',"ip='$ip'","dentist='$dentist', kine='$kine', ophtalmo='$ophtalmo', haircut='$haircut', esthetic='$esthetic', none='$none', other='$other', interest='$interest', price='$price', email='$email', comments='$comments'");
    return($noError);   
}

//PIERRE
// task: to update bddTable with form3 answer
function addForm3Answer($form,$ip){  
    $noError=update_Table_Condition_SetFieldsAndValues('stats',"ip='$ip'","mail='$form[mail]'");
    return($noError);   
}

//PIERRE
// task: to add new personal meeting id DB
//status=1 (published); status=0 (closed)
function addNewPersoMeet($form,$token){
    $title=addslashes($form['title']);
    $description=addslashes($form['description']);
    $place=addslashes($form['place']);
    $array_perso_meet=array('email'=>$form['email'],'title'=>$title,'description'=>$description,'place'=>$place,'web_url'=>$form['web_url'],'status'=>1,'token'=>$token);
    $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('perso_meet',$array_perso_meet);
    if($noError){
        $sql_res_array=sqlArray_OneResult_Tables_FieldsAsked_Condition('perso_meet','id',"title='".$title."' AND status='1' AND token='".$token."'");
        $m_id=$sql_res_array['id'];
        for($i = 0; $i < sizeof($form['date']);$i++){
            $array_perso_date=array('m_id'=>$m_id,'datetime'=>$form['date'][$i]['datetime'],'name'=>'','name_id'=>0);
            $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('perso_date',$array_perso_date);
            if(!$noError) return false;           
        }
        return true;   
    }
    else return false;
}

//PIERRE
/*return: 
 * 0 if email is missing
 * 1 if pwd... 
 * 2 if city...
 * 3 if new city proposed already exist
 * 4 if new city proposed cannot be added to bdd
 * 5 new city was added but user cannot
 * 6 all fine
 */
//BIG warning: we need to check if email already exist before subscription (not yet done)
function addNewUsers($form,$connection_id,$type){   
    if ($form['email']=="") $result= 0;
    elseif ($form['pwd']=="") $result= 1; 
    elseif ($form['city']=="other" and $form['new_city']=="") $result= 2; 
    else{ //normal work
        //to check if user use an already existing city or propose to create a new one
        if($form['new_city']=="") $city=addslashes($form['city']);
        else{ //to insert new city in BDD (with theme = 'Divers')
            $city=addslashes($form['new_city']);
            $array_city=array('region'=>'Midi Pyrénées','name'=>$city,);
            $result=sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('city',"name = '$city'",'region');
            //if city already exists
            if($result!==false) $result= 3;
            else $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('city',$array_city);
            if(!$noError) $result= 4;
        }
        $pwd_hash=hashString($form['pwd']); //hash password
        $array=array('type'=>$type,'email'=>$form['email'],'pwd'=>addslashes($pwd_hash),'city'=>addslashes($city));
        $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('user',$array);
        if($noError) $result= 6; // all right
        else $result= 5;
    }
    //log error as subscription is an important step to be carrefuly watched
    if ($result!=6) errorLog(5,'addNewUsers',"BIG WARNING: Error type=$result with connection_id=$connection_id in function addNewUsers");           
    return $result;
}

//PIERRE
// task: to update bddTable with RDV selection (if button was clicked)
function addRDVnum($num,$ip){
    $noError=update_Table_Condition_SetFieldsAndValues('stats',"ip='$ip'","num='$num'");
    return($noError);   
}

//PIERRE
// task: to update bddTable with visitor @IP
function addVisitor($ip,$city_page,$category){
    
    $date= date("Y-m-d-H-i-s");  
    $array=array('ip'=>$ip,'connection_datetime'=>$date,'city_page'=>$city_page,'category_page'=>addslashes($category),'actions'=>'CON_START-');
    $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('stats',$array);
    //$query1 = "insert into stats (login_datetime,email,ip) values ('$today','$email','$ip')";
    //$result1 = mysql_query($query1) or $noError=errorLog(5,'addVisitor',"Couldn't execute query1:$query1 of function addVisitor");                     
    $result1bis = mysql_query('SELECT @@IDENTITY AS connection_id');
    if(@mysql_num_rows($result1bis)>0) $row1bis = mysql_fetch_array($result1bis) or $noError=errorLog(5,'addVisitor',"Couldn't execute mysql_fetch_array('SELECT @@IDENTITY AS connection_id') of function addVisitor");                     
    else $noError=errorLog(5,'addVisitor',"Couldn't execute mysql_query('SELECT @@IDENTITY AS connectionId') of function addVisitor");                     
    if(!$noError) return false;
    $connection_id=$row1bis['connection_id'];         
    return($connection_id);
}

//PIERRE
// task: to build the categories array with all themes and subcategories
function buildCategoriesArray(){
    $categories_array['Tous'][0]="tous"; //initialisation
    $themes_array=sqlArray_Distinct_Field_Table_Condition_OrderKey_AscOrDesc('theme','category','','theme','ASC');
    foreach ($themes_array as $field=>$value){
        if($value!='Tous') $categories_array[$value]=sqlArray_Distinct_Field_Table_Condition_OrderKey_AscOrDesc('name','category',"WHERE theme=\"$value\"",'name','ASC');
    }
    return $categories_array;
    
}

//PIERRE
// task: to build the cities array with all regions and related cities
// exemple $cities_array['Midi Pyrénées'][0]='Toulouse'
function buildRegionsAndCitiesArray2D(){
    $cities_array2D=false; //initialisation
    $regions_array=sqlArray_Distinct_Field_Table_Condition_OrderKey_AscOrDesc('region','city','','id','ASC');
    foreach ($regions_array as $field=>$value){
        //$value=  addslashes($value);
        $cities_array2D[$value]=sqlArray_Distinct_Field_Table_Condition_OrderKey_AscOrDesc('name','city',"WHERE region=\"$value\"",'id','ASC');
    }
        return $cities_array2D;
}

//PIERRE
// task: to build the professional array according to user request (category and geographical situation)
// if parameter user_id is set, only one user info is return
//BIG WARNING: category parameter can be either a name of category or of theme
function buildProfessionalsArray2D($city,$category,$user_id,$key_words,$key_city,$page_nb,$max_result_per_pg,$my_slot_bool){
    $pro_array2D=false; //initialisation
    $offset=($page_nb-1)*$max_result_per_pg;
    //if parameter my_slot_bool is set, we are building real slotsArray (not fake) and then user shall have an email define to publish a slot. Thus filtering by email settings will split potential publishet from contact list (annuaire)
    if($my_slot_bool==true) $sql_cond="AND email!=''"; 
    else $sql_cond="AND 1=1";
    //add key words condition and check presence in all field of table user
    if ($key_words!='' and $key_words!='mots clé (option)'){
                
            $key_words_con='';
            $words_array = explode(' ',$key_words);//split keywords req in separated word
            foreach($words_array as $word)
            {
                  $key_words_con .= ' AND( title LIKE "%'.$word.'%" OR  subtitle LIKE "%'.$word.'%" OR category LIKE "%'.$word.'%" OR  label LIKE "%'.$word.'%" OR  city LIKE "%'.$word.'%" OR';
            }
            $key_words_con .= ' 1=0)';
        }
    else $key_words_con=' AND 1=1';
    //shall return only the array of the user_id
    if($user_id!=false){
        $pro_array2D=sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim('user',"*","type='1' AND status='1' AND id=\"$user_id\"",'title','ASC',$offset,$max_result_per_pg);    
    }
    elseif($category=='tous' or $category=='Tous'){ //to handle either selection of the category or thematic by the user
        $pro_array2D=sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim('user',"*","type='1' AND status='1' AND city=\"$city\"".$sql_cond.$key_words_con ,'title','ASC',$offset,$max_result_per_pg);    
    }
    else{ 
        if(sql_OneResult_Table_KeyField_KeyValue_FieldAsked('category', 'name', $category, 'theme')!=-1){ //parameter $category is a category type
            $pro_array2D=sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim('user',"*","type='1' AND status='1' AND category=\"$category\" AND city=\"$city\"".$sql_cond.$key_words_con,'title','ASC',$offset,$max_result_per_pg);  
        }//paramater $category is a theme type
        else $pro_array2D=sql_Joint_Cat_Theme($category,$city,$sql_cond,$key_words_con,$offset,$max_result_per_pg);        
    }
    return $pro_array2D;  
    
}

//PIERRE
/*return: 
 * 10 if city and new city not selected (city is missing)
 * 11 if title is missing
 * 12 if phone is missing
 * 13 if category is not set
 * 14 if new cat proposed already exist
 * 15 if new cat proposed cannot be added to bdd
 * 16 if new city proposed already exist
 * 17 if new city proposed cannot be added to bdd
 * 18 spare
 * 19 all fine
 */
function completeNewUsers($user_id,$form,$connection_id){  
    if ($form['city']=="other" and $form['new_city']=="") $result= 10;
    elseif ($form['title']=="") $result= 11;
    // 5 is kept as spare
    elseif ($form['phone']=="") $result= 12;
    elseif ($form['category']=="other" and $form['new_cat']=="") $result= 13;
    else{ //normal work
        //to check if user use an already existing category or propose to create a new one
        if($form['new_cat']=="") $category=$form['category'];
        else{ //to insert new cat in BDD (with theme = 'Divers')        
            $category=addslashes($form['new_cat']);
            $array_cat=array('theme'=>'Divers','name'=>$category,);
            $result=sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('category',"name = '$category'",'theme');
            if($result!==false) $result= 14;
            else $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('category',$array_cat);
            if(!$noError) $result= 15;
        }
        //to check if user use an already existing city or propose to create a new one
        if($form['new_city']=="") $city=addslashes($form['city']);
        else{ //to insert new city in BDD (with theme = 'Divers')
            $city=addslashes($form['new_city']);
            $array_city=array('region'=>'Midi Pyrénées','name'=>$city,);
            $result=sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('city',"name = '$city'",'region');
            //if city already exists
            if($result!==false) $result= 16;
            else $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('city',$array_city);
            if(!$noError) $result= 17;
        }
        //to get lat lng from google server (street and city are given as input)
        $sql_result=sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('user',"id = 'user_id'",'city');
        $city=$sql_result['city'];
        $string=$form['label']." ".$city;
        $lat_lng_array=getLatLng(addslashes($string));
        //if google did not find any lat lng, we put a random location near the city center
        if($lat_lng_array==false){
            $my_lat=0;
            $my_lng=0;
        }
        else{
            $my_lat=$lat_lng_array['latitude'];
            $my_lng=$lat_lng_array['longitude'];
        }
        //to prepare format of input
        $phone=addslashes($form['phone']);
        $title=addslashes($form['title']);
        $subtitle=addslashes($form['subtitle']);
        $label=addslashes($form['label']);
        //$category=addslashes($category);
        //BIG WARNING: lat_lng of user is updated with lat_lng of the last meeting place and not with the address of the user
        $noError=  update_Table_Condition_SetFieldsAndValues('user', "id='$user_id'", "category='$category' , phone='$phone', title='$title', subtitle='$subtitle', label='$label', latitude='$my_lat', longitude='$my_lng'");
        if($noError) $result= 19; // all right
        else $result= 18;
    }
    //log error as subscription is an important step to be carrefuly watched
    if ($result!=19) errorLog(5,'completeNewUsers',"BIG WARNING: Error type=$result with connection_id=$connection_id in function completeNewUsers");           
    return $result;
}


//PIERRE   
// task: to connect to BDD monrdvmalin
//either localhost or OVH
  
function connectDb(){
    $noError=true;

    $user="smartrdv";
    $host="smartrdv.mysql.db";
    $password="Evp2QxAm6ak7";
    $database = "smartrdv";     
    
    @$connection = mysql_connect($host,$user,$password) or $noError=errorLog(5,'connectDb',"couldn't connect to server");   
    if(!$noError){ // we are on local website and local bdd
        $user="root";
        $host="localhost";
        $password="malibufr";
        $database = "smartrdv";
        $connection = mysql_connect($host,$user,$password) or $noError=errorLog(5,'connectDb',"couldn't connect to server");   
    } 
    $db = mysql_select_db($database,$connection) or $noError=errorLog(5,'connectDb',"Couldn't select database"); 
    mysql_query("SET NAMES UTF8");
    return $noError;  
}



//to connect a user
/*return:
 *  0 if email doesn't exist in BDD
 *  1 if password is wrong
 *  2 if OK for login but stats update did not work
 *  3 if all OK
 */

function connectAndUpdateBdd($email,$password,$connection_id){
    $password_hash=hashString($password); //hash password
    $extractPassword=sql_OneResult_Table_KeyField_KeyValue_FieldAsked('pro','email',$email,'pwd');    
    // Checks if this login is already used by another member  
    if ($extractPassword == -1) return 0;
    // to check if the password is correct
    elseif ($extractPassword != $password_hash) return 1; 
    elseif ($extractPassword == $password_hash){  // login and password were found
        $today = date("Y-m-d H:i:s"); 
        if (update_Table_Condition_SetFieldsAndValues('stats', "connection_id='$connection_id'", "login_datetime='$today' , email='$email'")) return 3;
        else return 2;
        //updateBddOnesPerDay
        /*
         * If needed, put in the function what we have to do regularly to maintain BDD (purge, etc...)
         */ 
    }
    else return 0; // just in case
}//end function connect


//to del a slot
function deleteSlot($id){
    $result=update_Table_Condition_SetFieldsAndValues('meeting', "id='$id'", "status='0'"); 
}

//PIERRE
// task: to delete a row of a table
function delete_Table_KeyField_KeyValue($table,$keyField,$keyValue){
    $noError=true;
    $query = "DELETE FROM $table WHERE $keyField='$keyValue'";
    $result = mysql_query($query) or $noError=errorLog(5,'delete_Table_KeyField_KeyValue',"Couldn't execute query:$query of function delete_Table_KeyField_KeyValue"); 
    // return result
    return($noError); 
}

//PIERRE
// task: to delete a row of a table  
function delete_Table_KeyField1_KeyValue1_KeyField2_KeyValue2($table,$keyField1,$keyValue1,$keyField2,$keyValue2){
    $noError=true;
    $query = "DELETE FROM $table WHERE $keyField1='$keyValue1' AND  $keyField2='$keyValue2'";
    $result = mysql_query($query) or $noError=errorLog(5,'delete_Table_KeyField1_KeyValue1_KeyField2_KeyValue2',"Couldn't execute query:$query of function delete_Table_KeyField1_KeyValue1_KeyField2_KeyValue2"); 
    // return result
    return($noError);  
}


function sql_JointFieldsTablesConditions($fields,$table1,$table2,$cond,$order_key,$asc_desc,$offset,$max_result_per_pg){
    $noError=true;
    $array=null;
    $sql="SELECT $fields
    FROM $table1 
     INNER JOIN $table2
    ON $cond ORDER BY $order_key $asc_desc LIMIT $offset, $max_result_per_pg";
    //ChromePhp::log($sql);
    $result = mysql_query($sql) or $noError=errorLog(5,'sql_JointFieldsTablesConditions',"Couldn't execute sql:$sql ");       
    // return result
    if (@mysql_num_rows($result)>0 and $noError){
        $i=0;
        while($row=mysql_fetch_assoc($result)){
            $array[$i]=$row;
            $i+=1;
        }
        return ($array);
    } 
    else return (false);  
}

function sql_Joint_Cat_Theme($category,$city,$sql_cond,$key_words_con,$offset,$max_result_per_pg){
    $noError=true;
    $array=null;
    $sql="SELECT user.id, type, user.category,title,subtitle,email,phone,label,city,zip,country,latitude,longitude,web_url,description,status
    FROM user 
     INNER JOIN category
    ON user.category=category.name AND theme=\"$category\" AND type='1' AND status='1' AND city=\"$city\"".$sql_cond.$key_words_con." ORDER BY user.title ASC LIMIT $offset, $max_result_per_pg";
    //ChromePhp::log($sql);
    $result = mysql_query($sql) or $noError=errorLog(5,'sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim',"Couldn't execute sql:$sql ");       
    // return result
    if (@mysql_num_rows($result)>0 and $noError){
        $i=0;
        while($row=mysql_fetch_assoc($result)){
            $array[$i]=$row;
            $i+=1;
        }
        return ($array);
    } 
    else return (false);  
}

//PIERRE
// task: to build professionals list array according to the user request : in order to build fake slot (one fake slot is build per pro in order to display the pro contacts list using generic function for DrawRDV
//return array2D if at least one pro is found else return false
//if last param user_id is set (not false), return all slots of the user related to the search form (date & key_word)
// sql request is limited to slot > now() with status>0
function buildProListArray2D($city,$category,$key_words,$key_city,$page_nb,$max_result_per_pg){
    $offset= ($page_nb -1)*$max_result_per_pg;
    $slots_array2D=false; //initialisation
    //tomorrow=now +1days in order to build a fake slot after today (slot before today are not displayed) 
    $end_date=date("Y-m-d H:i:s",mktime(0, 0, 0, date("m")  , date("d")+1, date("Y")));
    //$my_slot_bool is set to false (fake slotsArray)
    $pro_array2D=buildProfessionalsArray2D($city,$category,false,$key_words,$key_city,$page_nb,$max_result_per_pg,false); // select compliant pro according to category and geographical situation
    if($pro_array2D!==false){
        $i=0;
        while(isset($pro_array2D[$i])){
            $slots_array2D[$i]['cal_selected']=false; //to keep track of slot selected in calendar box
            $slots_array2D[$i]['id']=$pro_array2D[$i]['id']; //fake slot id (substitute by user_id)
            list($date,$time)=explode(' ',$end_date);
            list($Y,$m,$d) = explode('-',$date);
            list($h,$min,$s) = explode(':',$time);
            $slot_date=datefr($date);// to convert date; ex: "2014-05-11" en "Dimanche 11 Mai"
            $slots_array2D[$i]['Y-m-d H:i:s']=$end_date; // to keep track on the standard SQL date format 
            $slots_array2D[$i]['date']=""; 
            $slots_array2D[$i]['hour']=$slots_array2D[$i]['end_hour']="";
            $slots_array2D[$i]['discount']='0';
            $slots_array2D[$i]['category']=$pro_array2D[$i]['category'];
            $slots_array2D[$i]['author_id']=$pro_array2D[$i]['id'];
            $slots_array2D[$i]['description']=$pro_array2D[$i]['description'];
            $slots_array2D[$i]['max_ticket_nb']=100;
            $slots_array2D[$i]['title']=$pro_array2D[$i]['title'];
            $slots_array2D[$i]['subtitle']=$pro_array2D[$i]['subtitle'];
            $slots_array2D[$i]['city']=$pro_array2D[$i]['city'];
            $slots_array2D[$i]['label']=$pro_array2D[$i]['label'];
            $slots_array2D[$i]['phone']=$pro_array2D[$i]['phone'];
            $slots_array2D[$i]['longitude']=$pro_array2D[$i]['longitude'];
            $slots_array2D[$i]['latitude']=$pro_array2D[$i]['latitude']; 
            $i+=1;
        }//end while
        return $slots_array2D;
    }//end if
    else return false;   
}

//PIERRE
// task: to build the slots array from our crawler (bot) according to the user request
//return array2D if at least one slot is found else return false
//if last param user_id is set (not false), return all slots of the user related to the search form (date & key_word)
// sql request is limited to slot > now() with status>0
function buildBotSlotsArray2D($author_id,$env,$city,$category,$key_words,$key_city,$from_date,$page_nb,$max_result_per_pg,$user_id){
    $noError=true;
    $sql="";
    $offset= ($page_nb -1)*$max_result_per_pg;
    $slots_array2D=$SQL_slots_array2D=$compliant_id_list=false; //initialisation
    $now=date("Y-m-d H:i:s");
    //end_date=now +31days
    $end_date=date("Y-m-d H:i:s",mktime(0, 0, 0, date("m")  , date("d")+31, date("Y")));
    //filter by author_id if given in parameter
    if($author_id>0){
        $sql_author_id_cond="AND target_id=$author_id"; //to filter by author_id (ex: page pro.php)
    }
    else $sql_author_id_cond=" AND 1=1 ";
    if($category=='tous' or $category=='Tous'){
        $sql_cat_cond="1=1"; //no condition on category
    }
    else{ //only one category or one thematic has been selected
        $category_id= sql_OneResult_Table_KeyField_KeyValue_FieldAsked('category', 'name', $category, 'id');
        // check if parameter given in search criteria is either a category or thematic 
        if($category_id==-1){ //thematic was given in parameter
            $cat_id_array=sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim('category', 'id', "theme='$category'", 'id', 'ASC', 0, 100);
            foreach ($cat_id_array as $field=>$value){
                $compliant_id_list.=$value['id'].",";
            }
            $compliant_id_list.="0";
        }
        else $compliant_id_list=$category_id; //category was given in parameter
        $sql_cat_cond="category IN ($compliant_id_list)";
    }
    //BIG WARNING: category is not part of the key words research --> to be added later on
    //add key words condition and check presence in all field of table meeting
    if ($key_words!='' and $key_words!='mots clé'){               
            $key_words_con='';
            $words_array = explode(' ',$key_words);//split keywords req in separated word
            foreach($words_array as $word)
            {
                  $key_words_con .= ' AND( title LIKE "%'.$word.'%" OR  menu LIKE "%'.$word.'%" OR submenu LIKE "%'.$word.'%" OR  label LIKE "%'.$word.'%" OR  city LIKE "%'.$word.'%" OR';
            }
            $key_words_con .= ' 1=0)';
        }
    else $key_words_con=' AND 1=1';
    $fields="m.id,m.meeting_date,m.meeting_end_date,t.category,t.info_url,t.booking_url,t.title,t.city,t.label,t.longitude,t.latitude,t.phone,m.discount_rate,s.discount_mode,s.price,s.target_id,s.submenu,s.menu,s.max_ticket_nb";
    switch($env) {
        case 0:{ // All environment 
            $sql = "SELECT $fields FROM bot_targets AS t INNER JOIN bot_submenus AS s ON t.id=s.target_id  INNER JOIN bot_meetings AS m ON m.submenu_id=s.id WHERE $sql_cat_cond AND city='$city' AND (TO_DAYS(meeting_date) - TO_DAYS('$from_date'))>=0 AND m.status>0 $key_words_con $sql_author_id_cond ORDER BY `m`.`meeting_date` LIMIT $offset, $max_result_per_pg";         
            break;
        }
        case 1:{ // DISCOUNT environment
            $sql = "SELECT $fields FROM bot_targets AS t INNER JOIN bot_submenus AS s ON t.id=s.target_id  INNER JOIN bot_meetings AS m ON m.submenu_id=s.id WHERE $sql_cat_cond AND city='$city' AND (TO_DAYS(meeting_date) - TO_DAYS('$from_date'))>=0 AND m.discount_rate>0 AND m.status>0 $key_words_con $sql_author_id_cond ORDER BY `m`.`meeting_date`,`m`.`discount_rate` LIMIT $offset, $max_result_per_pg";     
            break;
        }
        case 2:{ // EVENT environment
            /*$joint_cond=$sql_cat_cond." AND city='$city' AND (TO_DAYS(meeting_date) - TO_DAYS('$from_date'))>=0 AND status>0 $key_words_con.$sql_author_id_cond AND category.theme='Evènements' AND category.id=meeting.category";
            $SQL_slots_array_2D=sql_JointFieldsTablesConditions("*","meeting","category",$joint_cond,'meeting_date','ASC',$offset,$max_result_per_pg);*/
            break;
        }
        case 3:{ // PARTICULIERS environment
            $sql = "SELECT $fields FROM bot_targets AS t INNER JOIN bot_submenus AS s ON t.id=s.target_id  INNER JOIN bot_meetings AS m ON m.submenu_id=s.id INNER JOIN category AS c ON m.submenu_id=s.id WHERE $sql_cat_cond AND city='$city' AND (TO_DAYS(meeting_date) - TO_DAYS('$from_date'))>=0 AND m.status>0 $key_words_con $sql_author_id_cond ORDER BY `m`.`meeting_date` LIMIT $offset, $max_result_per_pg";         
            break;
            $joint_cond=$sql_cat_cond." AND city=$city AND (TO_DAYS(meeting_date) - TO_DAYS('$from_date'))>=0 AND status>0 $key_words_con.$sql_author_id_cond AND category.theme='Particuliers' AND category.id=meeting.category";
            $SQL_slots_array_2D=sql_JointFieldsTablesConditions("*","meeting","category",$joint_cond,'meeting_date','ASC',$offset,$max_result_per_pg);
            break;
        }
        default:{ //should not be possible
            $SQL_slots_array_2D=false;           
        }     
    } // end switch
    //execute sql query and get result
    //WARNING: DEBUG only
    ChromePhp::log($sql);
    $result = mysql_query($sql) or $noError=errorLog(5,'buildBotSlotsArray2D',"Couldn't execute sql:$sql ");
    if (@mysql_num_rows($result)>0 and $noError){
        $i=0;
        while($row=mysql_fetch_assoc($result)){
            $SQL_slots_array_2D[$i]=$row;
            $i+=1;
        }
    } 
    else $SQL_slots_array_2D=false; 

    $i=0;
    if($SQL_slots_array_2D){
        foreach ($SQL_slots_array_2D as $field=>$value){
            if($value['meeting_date']>$now){
                $slots_array2D[$i]['cal_selected']=false; //to keep track of slot selected in calendar box
                $slots_array2D[$i]['id']=$value['id'];
                list($date,$time)=explode(' ',$value['meeting_date']);
                list($Y,$m,$d) = explode('-',$date);
                list($h,$min,$s) = explode(':',$time);
                list($end_date,$end_time)=explode(' ',$value['meeting_end_date']);
                list($end_Y,$end_m,$end_d) = explode('-',$end_date);
                list($end_h,$end_min,$end_s) = explode(':',$end_time);
                $slot_date=datefr($date);// to convert date; ex: "2014-05-11" en "Dimanche 11 Mai"
                $category_name=sqlArray_OneResult_Tables_FieldsAsked_Condition('category','name',"id=".$value['category']);
                $slots_array2D[$i]['Y-m-d H:i:s']=$value['meeting_date']; // to keep track on the standard SQL date format 
                $slots_array2D[$i]['date']=$slot_date; 
                $slots_array2D[$i]['hour']=$h."h".$min;
                $slots_array2D[$i]['end_hour']=$end_h."h".$end_min;
                $slots_array2D[$i]['discount']=$value['discount_rate']; //BIG WARNING: applicable rate depend on discount_mode
                $slots_array2D[$i]['price']=$value['price'];
                $slots_array2D[$i]['category']=$category_name['name'];
                $slots_array2D[$i]['author_id']=$value['target_id'];
                $slots_array2D[$i]['description']=$value['submenu'];
                $slots_array2D[$i]['max_ticket_nb']=$value['max_ticket_nb'];
                $slots_array2D[$i]['info_url']=$value['info_url'];
                $slots_array2D[$i]['booking_url']=$value['booking_url'];    
                $slots_array2D[$i]['info_url']=$value['info_url'];  
                $slots_array2D[$i]['title']=$value['title'];
                $slots_array2D[$i]['subtitle']=$value['menu'];
                $slots_array2D[$i]['city']=$value['city'];
                if(isset($value['label'])) $slots_array2D[$i]['label']=$value['label'];
                else $slots_array2D[$i]['label']="";
                if(isset($value['phone'])) $slots_array2D[$i]['phone']=$value['phone'];
                else $slots_array2D[$i]['phone']="";
                $slots_array2D[$i]['longitude']=$value['longitude'];
                $slots_array2D[$i]['latitude']=$value['latitude']; 
                $slots_array2D[$i]['demo']=1;  
                $i=$i+1; 
            }
        }
        return $slots_array2D;
    }//endif
    else return false;
}

//PIERRE
// task: to build the slots array according to the user request
//return array2D if at least one slot is found else return false
//if last param user_id is set (not false), return all slots of the user related to the search form (date & key_word)
// sql request is limited to slot > now() with status>0
function buildSlotsArray2D($author_id,$env,$city,$category,$key_words,$key_city,$from_date,$page_nb,$max_result_per_pg,$user_id){
    $offset= ($page_nb -1)*$max_result_per_pg;
    $slots_array2D=$SQL_slots_array2D=$compliant_id_list=false; //initialisation
    $now=date("Y-m-d H:i:s");
    //end_date=now +31days
    $end_date=date("Y-m-d H:i:s",mktime(0, 0, 0, date("m")  , date("d")+31, date("Y")));
    //filter by author_id if given in parameter
    if($author_id>0){
        $sql_author_id_cond="AND author_id=$author_id"; //to filter by author_id (ex: page pro.php)
    }
    else $sql_author_id_cond=" AND 1=1 ";
    if($category=='tous' or $category=='Tous'){
        $sql_cat_cond="1=1"; //no condition on category
    }
    else{ //only one category or one thematic has been selected
        $category_id= sql_OneResult_Table_KeyField_KeyValue_FieldAsked('category', 'name', $category, 'id');
        // check if parameter given in search criteria is either a category or thematic 
        if($category_id==-1){ //thematic was given in parameter
            $cat_id_array=sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim('category', 'id', "theme='$category'", 'id', 'ASC', 0, 100);
            foreach ($cat_id_array as $field=>$value){
                $compliant_id_list.=$value['id'].",";
            }
            $compliant_id_list.="0";
        }
        else $compliant_id_list=$category_id; //category was given in parameter
        $sql_cat_cond="category IN ($compliant_id_list)";
    }
    //BIG WARNING: category is not part of the key words research --> to be added later on
    //add key words condition and check presence in all field of table meeting
    if ($key_words!='' and $key_words!='mots clé'){               
            $key_words_con='';
            $words_array = explode(' ',$key_words);//split keywords req in separated word
            foreach($words_array as $word)
            {
                  $key_words_con .= ' AND( title LIKE "%'.$word.'%" OR  subtitle LIKE "%'.$word.'%" OR description LIKE "%'.$word.'%" OR  label LIKE "%'.$word.'%" OR  city LIKE "%'.$word.'%" OR';
            }
            $key_words_con .= ' 1=0)';
        }
    else $key_words_con=' AND 1=1';
    switch($env) {
        case 0:{ // All environment
            $SQL_slots_array_2D=sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim('meeting',"*",$sql_cat_cond." AND city='$city' AND (TO_DAYS(meeting_date) - TO_DAYS('$from_date'))>=0 AND status>0 $key_words_con $sql_author_id_cond",'meeting_date','ASC',$offset,$max_result_per_pg);
            break;
        }
        case 1:{ // DISCOUNT environment
            $SQL_slots_array_2D=sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim('meeting',"*",$sql_cat_cond." AND city='$city' AND (TO_DAYS(meeting_date) - TO_DAYS('$from_date'))>=0 AND status>0 AND discount>0 $key_words_con $sql_author_id_cond",'meeting_date','ASC',$offset,$max_result_per_pg);
            break;
        }
        case 2:{ // EVENT environment
            $joint_cond=$sql_cat_cond." AND city='$city' AND (TO_DAYS(meeting_date) - TO_DAYS('$from_date'))>=0 AND status>0 $key_words_con.$sql_author_id_cond AND category.theme='Evènements' AND category.id=meeting.category";
            $SQL_slots_array_2D=sql_JointFieldsTablesConditions("*","meeting","category",$joint_cond,'meeting_date','ASC',$offset,$max_result_per_pg);
            break;
        }
        case 3:{ // PARTICULIERS environment
            $joint_cond=$sql_cat_cond." AND city=$city AND (TO_DAYS(meeting_date) - TO_DAYS('$from_date'))>=0 AND status>0 $key_words_con.$sql_author_id_cond AND category.theme='Particuliers' AND category.id=meeting.category";
            $SQL_slots_array_2D=sql_JointFieldsTablesConditions("*","meeting","category",$joint_cond,'meeting_date','ASC',$offset,$max_result_per_pg);
            break;
        }
        default:{ //should not be possible
            $SQL_slots_array_2D=false;           
        }     
    } // end switch
    $i=0;
    if($SQL_slots_array_2D){
        foreach ($SQL_slots_array_2D as $field=>$value){
            if($value['meeting_end_date']>$now){
                $slots_array2D[$i]['cal_selected']=false; //to keep track of slot selected in calendar box
                $slots_array2D[$i]['id']=$value['id'];
                list($date,$time)=explode(' ',$value['meeting_date']);
                list($Y,$m,$d) = explode('-',$date);
                list($h,$min,$s) = explode(':',$time);
                list($end_date,$end_time)=explode(' ',$value['meeting_end_date']);
                list($end_Y,$end_m,$end_d) = explode('-',$end_date);
                list($end_h,$end_min,$end_s) = explode(':',$end_time);
                $slot_date=datefr($date);// to convert date; ex: "2014-05-11" en "Dimanche 11 Mai"
                $category_name=sqlArray_OneResult_Tables_FieldsAsked_Condition('category','name',"id=".$value['category']);
                $slots_array2D[$i]['Y-m-d H:i:s']=$value['meeting_date']; // to keep track on the standard SQL date format 
                $slots_array2D[$i]['date']=$slot_date; 
                $slots_array2D[$i]['hour']=$h."h".$min;
                $slots_array2D[$i]['end_hour']=$end_h."h".$end_min;
                $slots_array2D[$i]['discount']=$value['discount'];
                $slots_array2D[$i]['price']=$value['price'];
                $slots_array2D[$i]['category']=$category_name['name'];
                $slots_array2D[$i]['author_id']=$value['author_id'];
                $slots_array2D[$i]['description']=$value['description'];
                $slots_array2D[$i]['max_ticket_nb']=$value['max_ticket_nb'];
                $slots_array2D[$i]['info_url']=$value['info_url'];
                $slots_array2D[$i]['booking_url']=$value['booking_url'];    
                $slots_array2D[$i]['info_url']=$value['info_url'];  
                $slots_array2D[$i]['title']=$value['title'];
                $slots_array2D[$i]['subtitle']=$value['subtitle'];
                $slots_array2D[$i]['city']=$value['city'];
                if(isset($value['label'])) $slots_array2D[$i]['label']=$value['label'];
                else $slots_array2D[$i]['label']="";
                if(isset($value['phone'])) $slots_array2D[$i]['phone']=$value['phone'];
                else $slots_array2D[$i]['phone']="";
                $slots_array2D[$i]['longitude']=$value['longitude'];
                $slots_array2D[$i]['latitude']=$value['latitude']; 
                $slots_array2D[$i]['demo']=$value['demo'];  
                $i=$i+1; 
            }
        }
        //ChromePhp::log($slots_array2D);
        return $slots_array2D;
    }//endif
    else return false;
}

//PIERRE
// task: get coordinates of one city given in parameter
function getCoordinatesCity($city){
    $coordinates=false; //initialisation
    $coordinates=sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('city',"name = '$city'",'longitude,latitude');
    return $coordinates;
}


//to get latitude and longitude from city and street given in parameter
function getLatLng($string){
   $string = str_replace (" ", "+", urlencode($string));
   $ch = curl_init();
   $details_url = "http://maps.googleapis.com/maps/api/geocode/json?address=".$string."&sensor=false";
   curl_setopt($ch, CURLOPT_URL, $details_url);
   curl_setopt($ch, CURLOPT_HEADER, 0);
   
   curl_setopt($ch, CURLOPT_USERAGENT, $_SERVER['HTTP_USER_AGENT']);
   curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
   curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
   
   $response = json_decode(curl_exec($ch), true);
   // If Status Code is ZERO_RESULTS, OVER_QUERY_LIMIT, REQUEST_DENIED or INVALID_REQUEST
   if ($response['status'] != 'OK') {
        //ChromePhp::log($response['status']);
        return false;
   }
 
   $geometry = $response['results'][0]['geometry'];
 
    $latitude = $geometry['location']['lat'];
    $longitude = $geometry['location']['lng'];
 
    $array = array(
        'latitude' => $geometry['location']['lat'],
        'longitude' => $geometry['location']['lng'],
        'location_type' => $geometry['location_type'],
    );
    if(isset($array) and sizeof($array)>0){
        $result['latitude']=$array['latitude'][0];
        $result['longitude']=$array['longitude'][0];
        return $array;
    }
    else return false;
}


//PIERRE
// task: to get all data of a personal meeting
/*
-- id   
-- email   
-- title  
-- description 
-- place   
-- web_url 
-- status  
-- token
-- dates
------- [i]
----------- datetime
----------- name
*/
function getPersoMeet($token){
    $current_datetime=''; // to initiate varibale
    $i=0;
    $sql_res_array1=sqlArray_OneResult_Tables_FieldsAsked_Condition('perso_meet','*',"token='".$token."'");
    if($sql_res_array1['id']!=null){
        $names_id_array=array();
        $sql_res_array2=sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim('perso_date', 'datetime, name,id,name_id', "m_id='".$sql_res_array1['id']."'", 'datetime', 'ASC', 0, 50);
        foreach ($sql_res_array2 as $key => $value) {
            if($value['name']!=='' && array_search($value['name'], $names_id_array)===false){ // test if name is not already in the array
                $names_id_array[$value['name_id']]=$value['name'];
            }
            if($value['datetime']===$current_datetime && $value['datetime']!=='0000-00-00 00:00:00'){
                $dates[$i-1][$current_datetime][$j]=$value['name'];
                $j++; //increase key_name
            }
            else if($value['datetime']!=='0000-00-00 00:00:00'){
                $j=0; // restart key_name for new datetime
                $current_datetime=$value['datetime']; // update current datetime
                $dates[$i][$current_datetime][$j]=$value['name'];
                $i++; //increase key_datetime
                $j++; //increase key_name
            }
        }
        if($dates[0]!=null || $dates[0]!='0000-00-00 00:00:00'){
            $sql_res_array1['dates']=$dates;
            $sql_res_array1['names_id']=$names_id_array;
            return $sql_res_array1;
        } 
        else return false;  
    }
    else return false;
}

//PIERRE
// task: insert a line into a BDD table
function insertInBdd_Table_SpecifiedFieldsAndArrayValues($table,$array){
    $noError=true;
    $fields="";
    $values="";
    $i=0; 
    foreach ($array as $field=>$value){
        if ($i==0){
        $fields.=$field;
        $values.=$value;
        }
        else{
        $fields.=",".$field;
        $values.="','".$value;
        }
        $i=$i+1;
    }
    $query1 = "insert into $table ($fields) values ('$values')";
    ChromePhp::log($query1);
    $result1 = mysql_query($query1) or $noError=errorLog(5,'insertInBdd_Table_SpecifiedFieldsAndArrayValues',"Couldn't execute query1:$query1 of function insertInBdd_Table_SpecifiedFieldsAndArrayValues");                                         
    // return result
    return($noError);
}
/* To publish a new slot
 * return 0 if visitor is not login 
 * 1 if no start date
 * 2 if no start time
 * 3 if no end time
 * 4 if no title
 * 5 if no city
 * 6 if suggested new city exists already
 * 7 if do not already exist but is not well formatted and cannot be added in bdd
 * 8 if no category
 * 9 if suggested new category exists already
 * 10 if do not already exist but is not well formatted and cannot be added in bdd
 * 11 if no phone
 * 12 form was OK but cannot insert slot in BDD
 * 13 all fine!
 */
function publishNewSlot($form,$user_id){ 
    $noError=true;
    $time_now=date("Y-m-d-H-i-s");
    if($user_id!=$_SESSION['user_id']) return 0;
    elseif ($form['new_slot_start_date']=="") return 1;
    elseif ($form['new_slot_start_time']=="") return 2; 
    elseif ($form['new_slot_end_time']=="") return 3;
    elseif ($form['title']=="") return 4;
    elseif ($form['city']=="other" and $form['new_city']=="")  return 5;
    elseif ($form['category']=="other" and $form['new_cat']=="")  return 8; 
    elseif ($form['phone']=="") return 11;
    else{ //normal work
        if($form['city']!="other" and $form['new_city']=="") $city=addslashes($form['city']);
        else{ //to insert new city in BDD (with theme = 'Divers')
            $city=addslashes($form['new_city']);
            $array_city=array('region'=>'Midi Pyrénées','name'=>$city);
            $result=sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('city',"name = '$city'",'region');
            //if city already exists
            if($result!==false) return 6;
            else $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('city',$array_city);
            if(!$noError) return 7;
        }
        if($form['category']!="other" and $form['new_cat']=="") $category=addslashes($form['category']);
        elseif($form['new_cat']!=""){ //to insert new cat in BDD (with theme = 'Divers')           
            $category=addslashes($form['new_cat']);
            $array_cat=array('theme'=>'Divers','name'=>$category,);
            $result=sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('category',"name = '$category'",'theme');
            if($result!==false) return 9;
            else $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('category',$array_cat);
            if(!$noError) return 10;
        } 
        //BIG WARNING: label of the meeting is used to define label of the user (not always true)
        if($form['label']!=""){
            $label=addslashes($form['label']);
            $string=$form['label']." ".$city;
            $lat_lng_array=getLatLng(addslashes($string));
            //if google did not find any lat lng, we put a random location near the city center
            if($lat_lng_array==false){
                $city_lat_lng_array=  sqlArray_OneResult_Tables_FieldsAsked_Condition('city', 'latitude,longitude', "name=\'$city\'");
                $my_lat=$city_lat_lng_array['latitude'];
                $my_lng=$city_lat_lng_array['longitude'];
            }
            else{
                $my_lat=$lat_lng_array['latitude'];
                $my_lng=$lat_lng_array['longitude'];
            }
        }
        else{ // if label not completed, we use default coordinates of city
            $label="";
            $city_lat_lng_array=  sqlArray_OneResult_Tables_FieldsAsked_Condition('city', 'latitude,longitude', "name='$city'");
            $my_lat=$city_lat_lng_array['latitude'];
            $my_lng=$city_lat_lng_array['longitude'];
        }
        $meeting_date=$form['new_slot_start_date']." ".$form['new_slot_start_time'].":00";
        $meeting_end_date=$form['new_slot_start_date']." ".$form['new_slot_end_time'].":00";
        $sql_result=sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('category',"name = '$category'",'id');
        $my_cat_id=$sql_result['id'];
        $phone=addslashes($form['phone']);
        $title=addslashes($form['title']);
        $subtitle=addslashes($form['subtitle']);
        $description=addslashes($form['description']);
        $booking_url=addslashes($form['booking_url']);
        $array=array('author_id'=>$user_id ,'phone'=>$phone, 'city'=>$city, 'category'=>$my_cat_id, 'title'=>$title, 'subtitle'=>$subtitle, 'creation_date'=>$time_now, 'meeting_date'=>$meeting_date,'meeting_end_date'=>$meeting_end_date, 'label'=>$label , 'latitude'=>$my_lat, 'longitude'=>$my_lng, 'discount'=>$form['discount'],'description'=>$description,'booking_url'=>$booking_url,'max_ticket_nb'=>$form['max_ticket_nb']);
        $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('meeting',$array);
        if($noError) return 13; // all right
        else return 12;
    }
    //log error as publication is an important step to be carrefuly watched
    if ($result!=13) errorLog(5,'publishNewSlot',"BIG WARNING: Error type=$result with user_id=$user_id in function publishNewSlot");           
    return $result;
}

//PIERRE
// task: to return several rows of a table
function sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim($tables,$fieldsAsked,$condition,$orderKey,$ascOrDesc,$offset,$limit){
    $noError=true;
    $array=null;
    $sql = "SELECT $fieldsAsked FROM $tables WHERE $condition ORDER BY $orderKey $ascOrDesc LIMIT $offset, $limit";
    //ChromePhp::log($sql);
    $result = mysql_query($sql) or $noError=errorLog(5,'sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim',"Couldn't execute sql:$sql ");       
    // return result
    if (@mysql_num_rows($result)>0 and $noError){
        $i=0;
        while($row=mysql_fetch_assoc($result)){
            $array[$i]=$row;
            $i+=1;
        }
        return ($array);
    } 
    else return (false);     
}

//PIERRE
//task: to get all distinct fields of table
function sqlArray_Distinct_Field_Table_Condition_OrderKey_AscOrDesc($fieldAsked,$table,$condition,$key_order,$orderSQL){
    $array=false;
    $noError=true;
    $i=0;
    $sql = "SELECT DISTINCT $fieldAsked FROM $table $condition ORDER BY $table.$key_order $orderSQL";
    $result = mysql_query($sql) or $noError=errorLog(5,'sql_Distinct_Field_Table_Condition',"Couldn't execute sql:$sql of function sql_Distinct_Field_Table_Condition");
    while(@mysql_num_rows($result)>0 and $row = mysql_fetch_assoc($result)){
        $array[$i]=current($row); //to return only first element of the array
        $i=$i+1;
    }       
    if ($noError) return($array);
    else return false;
}// end function sql_Distinct_Field_Table_Condition

//PIERRE
//task: check if one row exist in the table
function sqlArray_IsThereOneResult_Table_Condition_FieldsAsked($table,$condition,$fieldsAsked){
    $sql = "SELECT $fieldsAsked FROM $table WHERE $condition";   
    $result = mysql_query($sql);
    if(@mysql_num_rows($result)>0 and $row=mysql_fetch_assoc($result)) return $row;
    else return false;   
}

//PIERRE
// task: to return one row of a table
function sqlArray_OneResult_Tables_FieldsAsked_Condition($tables,$fieldsAsked,$condition){
    $noError=true;
    $sql = "SELECT $fieldsAsked FROM $tables WHERE $condition";
    //ChromePhp::log($sql);
    $result = mysql_query($sql) or $noError=errorLog(5,'sqlArray_OneResult_Tables_FieldsAsked_Condition',"Couldn't execute sql:$sql ");
    if(@mysql_num_rows($result)>0 and $row = mysql_fetch_assoc($result) and $noError) return $row; 
    else return (-1);      
}

//PIERRE
// task: to count nb of results of a sql query
function sql_NbResults($tables,$condition){
    $noError=true;
    $sql = "SELECT COUNT(*) FROM $tables WHERE $condition";
    //ChromePhp::log("sql_NbResults query: ".$sql);
    $result = mysql_query($sql) or $noError=errorLog(5,'sql_NbResults',"Couldn't execute sql:$sql ");
    if($noError){
        return mysql_result($result,0); 
    } 
    else return (0); 
}

//PIERRE
// task: to return one field of a table
function sql_OneResult_Table_KeyField_KeyValue_FieldAsked($table,$keyField,$keyValue,$fieldAsked){
    $noError=true;
    $row=false;
    $sql = "SELECT $fieldAsked FROM $table WHERE $keyField='$keyValue'";
    //ChromePhp::log($sql);
    $result = mysql_query($sql) or $noError=errorLog(5,'sql_OneResult_Table_KeyField_KeyValue_FieldAsked',"Couldn't execute sql:$sql of function sql_OneResult_Table_KeyField_KeyValue_FieldAsked");
    if($result and @mysql_num_rows($result)>0) $row = mysql_fetch_array($result) or $noError=errorLog(5,'sql_OneResult_Table_KeyField_KeyValue_FieldAsked',"Couldn't execute mysql_fetch_array:$sql of function sql_OneResult_Table_KeyField_KeyValue_FieldAsked");
    else $noError=errorLog(5,'sql_OneResult_Table_KeyField_KeyValue_FieldAsked',"Couldn't execute mysql_fetch_array:$sql of function sql_OneResult_Table_KeyField_KeyValue_FieldAsked"); 
    // return result
    if ($noError and $row) return ($row[$fieldAsked]);
    else return (-1); // notice: This function return -1 instead of 0 because this function return number and could use 0.             
}


function update_lat_lng_bdd(){
    $no_lat_in_bdd=  sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim('meeting', '*', "latitude<=0", 'id', 'ASC', 0, 1000);
    if ($no_lat_in_bdd==false) return true;
    $i=0;
    foreach ($no_lat_in_bdd as $key => $value) {
            $string=$value['label']." ".$value['city'];
            //ChromePhp::log($string);
            $lat_lng_array=getLatLng($string);
            $my_lat=$lat_lng_array['latitude'];
            $my_lng=$lat_lng_array['longitude'];
            $my_id=$value['id'];
            update_Table_Condition_SetFieldsAndValues('meeting', "id=$my_id", "latitude='$my_lat' , longitude='$my_lng'");                                
        }
    return true;
}


//PIERRE
function updatePartners($form,$connection_id,$datetime){
    $previous_actions=sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('stats', "connection_id='$connection_id'", 'actions');
    if(isset($previous_actions['actions'])) $actions=$previous_actions['actions']."NEW PARTNER";
    update_Table_Condition_SetFieldsAndValues('stats',"connection_id='$connection_id'","actions='$actions'");
    $name=addslashes($form['name']);
    $partner_array=array('email'=>$form['email'],'name'=>$name,'connection_id'=>$connection_id,'datetime'=>$datetime);
    insertInBdd_Table_SpecifiedFieldsAndArrayValues('partners',$partner_array);
}
/* used with fake slot for debugging purpose
//PIERRE
// task: to update the date of slots once a day (until real slot will be created by professionals)
// info: this task is executed only at the first visit of the day
function updateSlotsDate($ip){
    $today=date("Y-m-d");
    $time=date("Y-m-d-H-i-s");
    $limit=1000;
    //check if first connexion of the day
    if (sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('stats',"DATE(connexion_date) = DATE(NOW())",'ip')) 
        {return false;} 
    else{ // else update all dates of slots in BDD 
        $date1=dateNextWorkingDayAfterDate($today); // next working day
        $date2=dateNextWorkingDayAfterDate(dateAddDays($date1,1)); // in 2 working days
        $date3=dateNextWorkingDayAfterDate(dateAddDays($date2,1)); // in 3 working days
        //list of meeting id for each date
        $slot1=array(1,2,3,4,13,14,23,24,33,34,44,45,46,55,56,65,66,76,77,78,87,88,97,98,108,109,110,119,120);
        $slot2=array(5,6,7,15,16,17,25,26,27,35,36,37,43,47,48,49,57,58,59,67,68,69,75,79,80,81,89,90,91,99,100,101,107,111,112,113,121,122,123);
        $slot3=array(8,9,10,11,12,18,19,20,21,22,28,29,30,31,32,38,39,40,41,42,50,51,52,53,54,60,61,62,63,64,70,71,72,73,74,82,83,84,85,86,92,93,94,95,96,102,103,104,105,106,114,115,116,117,118,124,125,126,127,128);
        //get all meetings
        $SQL_slots_array_2D=sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim('meeting','id , meeting_date',"id!=0",'meeting_date','ASC',0,$limit);
        foreach ($SQL_slots_array_2D as $field=>$value){
            $id=$value['id'];
            // explode time to keep it as is
            list($date,$time)=explode(' ',$value['meeting_date']);
            // change days according respective delay between initial slots
            if (in_array($id,$slot1)) update_Table_Condition_SetFieldsAndValues('meeting',"id=$id","meeting_date='".$date1.' '.$time."', creation_date=NOW()"); 
            elseif (in_array($id,$slot2)) update_Table_Condition_SetFieldsAndValues('meeting',"id=$id","meeting_date='".$date2.' '.$time."', creation_date=NOW()");
            elseif (in_array($id,$slot3)) update_Table_Condition_SetFieldsAndValues('meeting',"id=$id","meeting_date='".$date3.' '.$time."', creation_date=NOW()");     
        }
        return true;             
    }
}
*/

//PIERRE
// task: to update all data of a personal meeting (add,delete,modify user's datetime)
function updatePersoMeet($form,$token){
    ChromePhp::log("updatePersoMeet");
    ChromePhp::log($form);
    $sql_res_array=sqlArray_OneResult_Tables_FieldsAsked_Condition('perso_meet','id',"status='1' AND token='".$token."'");
    $m_id=$sql_res_array['id'];
    $names_without_dates_array=$form["names_id"]; //initialisation of array of names that do not participate to any dates of the meeting
    $i=0;
    delete_Table_KeyField_KeyValue('perso_date','m_id',$m_id); // to delete old dates info regarding this meeting
    foreach ($form["dates"] as $keydate => $value) { //insert perso dates
        foreach ($value as $datetime => $names_array) {
            foreach ($names_array as $key => $name) {
/*                if(array_search($name, $form["names_id"])!==false){// we get the name_id provided by js code
                    $name_id=array_search($name, $form["names_id"]);
                } */
                unset($names_without_dates_array[array_search($name, $form["names_id"])]); // remove this name from array of names that do not participate to any dates of the meeting
                if(!array_search($name, $form["names_id"])){
                    $name_id=0;
                }
                else $name_id=array_search($name, $form["names_id"]);
                $array_perso_date=array('m_id'=>$m_id,'datetime'=>$datetime,'name'=>$name,'name_id'=>$name_id);
                ChromePhp::log($array_perso_date);
                $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('perso_date',$array_perso_date); // to update old dates info regarding this meeting
                ChromePhp::log("Noerror: ".$noError);
                if(!$noError) return false; 
            }  
        }            
    }
    foreach ($names_without_dates_array as $key => $value) { //to keep track on user that answer to the poll without participating to any dates
        $array_perso_date=array('m_id'=>$m_id,'datetime'=>null,'name'=>$value,'name_id'=>$key);
        $noError=insertInBdd_Table_SpecifiedFieldsAndArrayValues('perso_date',$array_perso_date); 
        if(!$noError) return false; 
    }
    return true;
}


//PIERRE
function updateStats($connection_id,$actions){
    $previous_actions=sqlArray_IsThereOneResult_Table_Condition_FieldsAsked('stats', "connection_id='$connection_id'", 'actions');
    if(isset($previous_actions['actions'])) $actions=$previous_actions['actions'].$actions;
    update_Table_Condition_SetFieldsAndValues('stats',"connection_id='$connection_id'","actions='$actions'");
}

//PIERRE
// task: to update bddTable following condition in parameter
function update_Table_Condition_SetFieldsAndValues($table,$condition,$setFieldsAndValues){
    $noError=true;
    $sql = "UPDATE $table SET $setFieldsAndValues WHERE  $condition";
    //ChromePhp::log($sql);
    $result = mysql_query($sql) or $noError=errorLog(5,'update_Table_Condition_SetFieldsAndValues',"Couldn't execute sql:$sql of function update_Table_Condition_SetFieldsAndValues");
    // return result
    return $noError;
}

function setPageTitleSylvain($title) {
    echo "smartRDV.fr | $title";
}



/******************************************************************************************************************************/
/******************************************************************************************************************************/
/******************************************************************************************************************************/

/* Functions Sylvain */

/******************************************************************************************************************************/
/******************************************************************************************************************************/
/******************************************************************************************************************************/


function connectAccount(){

}


function updateUserData($user_id, $array_data){
    $noError = true;
    $sql = "UPDATE pro SET ".arrayToSqlFields($array_data)." WHERE  id = ".$user_id;
    $result = mysql_query($sql) or $noError = errorLog(5,'updateUserData',"Couldn't execute sql: $sql");
    return $noError;}

function isRigthStringFormat($string, $type){ // Return true or string = error message
    switch ($type){
        case 'email':
            if($string == '') return "Veuillez saisir l'email.";
            if(preg_match("/@.*\./",$string) == 0) return "L'email n'est pas valide.";
            return true; break;
        case 'password':
            if($string == '') return "Veuillez saisir le mot de passe.";
            return true; break;
        case 'code postal':
            if($string == '') return "Veuillez saisir le code postal.";
            return true; break;
        case 'categorie':
            if($string == '') return "Veuillez saisir la catégorie.";
            return true; break;
        case 'adress':
            if($string == '') return "Veuillez saisir l'adresse.";
            return true; break;
        case 'url':
            if($string == '') return "Veuillez saisir l'url.";
            return true; break;
        case 'tel':
            if($string == '') return "Veuillez saisir le téléphone.";
            return true; break;
        case 'entreprise':
            if($string == '') return "Veuillez saisir le nom de l'entreprise.";
            return true; break;
        default: return true;}}

function getDatabaseData($table,$fieldsAsked,$condition){return sqlArray2D_AllRows_Tables_FieldsAsked_Condition_OrderKey_AscOrDesc_Off_Lim($table, $fieldsAsked, $condition, 'id', 'ASC', 0, 500);}

function arrayToSqlFields($array){
    $first = true;
    $sql = "";
    foreach ($array as $field => $value){
        if(is_string($value)) $value = "'".$value."'";
        if($first == true ){
        $first = false;
        $sql.= $field.' = '.$value;}
        else{
        $sql.= ','.$field.' = '.$value;}}
    return $sql;}











?>