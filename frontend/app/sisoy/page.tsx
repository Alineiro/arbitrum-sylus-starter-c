"use client"

import { useState } from "react"

export default function Hola42() {
	const [input, setInput] = useState("")

	function pingpong() {
		if (input == "ping") {
			alert("pong")
		}
		else {
			alert("ping");
		}
	}

	return (
		<>
			<div
				style={{
					backgroundColor: input,
				}}
				className="bg-red-600 text-black p-12"
			/>
			<input 	onChange={(event) => {
					setInput(event.target.value)
				}}
				className="border-2 text-white border-black bg-black p4 rounded" 
				placeholder="Opina sobre Macondo"/>
			
			<button onClick={pingpong}>
				<span className="text-black">Enviar</span>
			</button>
			<Gatito />
			<Gatito />
			<Gatito />
			<Gatito />
		</>
	)
}

function Gatito() {
	return (
		<div>,
		<h2>Patatas</h2>,
		<img 
			onClick={() => alert("Macondo")}
			src="https://imgs.search.brave.com/9zU6fEQTWzpCnwBklcDYacro_PjQDsd8RU-kVnffIxE/rs:fit:500:0:0:0/g:ce/aHR0cHM6Ly93d3cu/ZW50cmVlc3R1ZGlh/bnRlcy5jb20vd3At/Y29udGVudC91cGxv/YWRzLzIwMjIvMDIv/SkFHNjM0MC5qcGc"/>,
		<p>Miaw mamaguevo</p>,
		</div>
	)
}
