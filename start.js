var back_button = document.querySelector("#back_icon");
back_button.addEventListener("click",()=>{
    window.location.href = "index.html";
});


//from here arrangement code begins....

import { initializeApp } from 'firebase/app';
import { getDatabase, ref, onValue, set, once, update} from "firebase/database";

const firebaseConfig = {
  databaseURL: "https://sparkbot-ee008-default-rtdb.firebaseio.com/",
};

const app = initializeApp(firebaseConfig)
const database = getDatabase(app);
const data1Ref = ref(database, 'Roverdata/distance')

var container = document.querySelector(".container");

onValue(data1Ref, (snapshot) => {
    const dist = snapshot.val();

    //creating a div..
    var newDiv = document.createElement("div");
    //giving it class "items" ..
    newDiv.classList.add('items');

    //checking availability of new div and entering data..
    if (newDiv) {
        newDiv.innerHTML = dist;
    } else {
        console.error("Element with id 'newdiv' not found.")
    }

    container.appendChild(newDiv);
})


