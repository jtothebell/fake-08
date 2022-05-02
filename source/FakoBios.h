#pragma once

#include <string>

#ifndef VER_STR
#define VER_STR "v0.0.0.0"
#endif

//from PicoLove api.lua

const char * fake08BiosP8 = R"#(
pico-8 cartridge // http://www.pico-8.com
version 35
__lua__
--all themes

--start customizable per platform
cartpath = "sdmc:/p8carts/"
selectbtn = "a"
pausebtn = "start"
versionstr = ")#" VER_STR  R"#("
exitbtn = "l + r"
sizebtn = "select to cycle screen sizes"
--end customizable per platform

darkenpal = {0,0,1,1,5,5,13,5,5,13,13,13,1,13,13}
darkenpal[0] = 0
-->8
--classic

function classic_init()

	carts={}
	numcarts = 0
	
	usestring = true
	
	if __getsetting then
			pal(1, __getsetting('p8_bgcolor'))
			pal(7, __getsetting('p8_textcolor'))
	else
		carts = {'cart1.p8','cart2.p8.png','thirdcart.p8'}
		numcarts = 3
		usestring = false
	end
	
	
	cidx=0
	carttoload = ""
	t=0
	linebuffer=""
	
	bgcolor=1
	runcmd=false

	if __listcarts then
		carts = __listcarts()
		numcarts = #carts
	end

	if __getbioserror then
		linebuffer = __getbioserror()
	end
end

function classic_update()
	t+=1
	if btnp(1) then
		cidx = min((cidx + 1), numcarts)
	end
	if btnp(0) then
		cidx = max((cidx - 1), 1)
	end
	
	if btnp(2) then
		cidx = max((cidx - 10), 1)
	end
	if btnp(3) then
		cidx = min((cidx + 10), numcarts)
	end

	if btnp(4) then
		linebuffer = ""
	end
	if btnp(5) then
		linebuffer = ""
		runcmd = true
	end
	
	if cidx > 0 and cidx <= numcarts then
		carttoload = carts[cidx]
		local lastslashidx = nil
		if usestring then
			lastslashidx = string.find(carttoload, "/[^/]*$")
		end
		local dispstr = carttoload
		if lastslashidx ~= nil and lastslashidx > 0 then
			dispstr = sub(dispstr, lastslashidx + 1)
		end
		linebuffer = "load " .. dispstr
	else
		carttoload = ""
	end
	
	if runcmd then
		--make call to global
		--load cart here
		
		if load and carttoload then
			load(carttoload)
		end
	end
		
end

function classic_draw()
	cls()
	rectfill(0,0,128,128,bgcolor)
	spr(128, 1, 5, 6, 1)
	color(7)
	cursor(0, 6*3)
	print("welcome to fake-08")
	print("a homebrew pico-8 emulator")
	print("currently in alpha (" .. versionstr .. ")")
	print("")
	print("place p8 carts in " .. cartpath)
	if numcarts < 1 then
		print("--no carts found--")
	else
		print("⬅️➡️ to navigate carts 1 by 1")
		print("⬆️⬇️ to navigate carts 10 by 10")
		print("🅾️ (" .. selectbtn .. ") to load selected cart")
		print(pausebtn .. " to close current cart")
	end
	print(sizebtn)
	print(exitbtn .. " to exit")
	print("")
	-- 18 pixels from cursor() call, then 
	-- 24 more from 4 print calls
	linenum=84


	rectfill(0, linenum, 128, linenum+5, bgcolor)
	color(7)
	print("> "..linebuffer, 0, linenum)
	if t%30<15 then
		rectfill((#linebuffer+2)*4, linenum, (#linebuffer+2)*4+3, linenum+4, 8)
	end
end

-->8
--fancy

function fancy_init()
	carts={}
	numcarts = 0
	
	usestring = true
	
	showerror = false
	bioserror = ''
	errortable = {'error! press ❎ to close.'}
	if __getbioserror then
		bioserror = __getbioserror()
	end
	
	if bioserror != '' then
		showerror = true
		while true do
			if #bioserror >= 26 then
				add(errortable,sub(bioserror,1,26))
				bioserror = sub(bioserror,27,-1)
			else
				add(errortable,bioserror)
				break
			end
		end
	end
	
	if __getsetting then
			bgcolor = __getsetting('p8_bgcolor')
			textcolor = __getsetting('p8_textcolor')
	else
		bgcolor=1
		textcolor = 7
	end
	
	
	cidx=0
	carttoload = ""
	t=0
	linebuffer=""
	
	
	runcmd=false
	
	cpos = 0
	dcpos = 0
	loadedimages = {}
	if __listcarts then
		cartnames = __listcarts()
		numcarts = #cartnames
	else
		--cartnames = {'cart1.p8','cart2.p8.png','thirdcart.p8','cart4.p8','5cart.p8','c6.p8','theseventhcart.p8.png'}
		cartnames = {}
		numcarts = #cartnames
		usestring = false
	end
	
	loading = true
	
	loadtotal = #cartnames
	loadnumber = 1
	

	


end

function fancy_updatecarts()
	for i,v in ipairs(carts) do
		if v.id >= cpos -1 and v.id <= cpos+1 then
			v.visible = true
			
			if loadedimages[v.sprite] ~= v.name then
				loadedimages[v.sprite] = v.name
				if __loadlabel then
					__loadlabel(v.name,true,v.sprite*16)
				end
			end
		
		else
			v.visible = false
		end
		if v.id == cpos then
			carttoload = v.name
		end
	end
end

function fancy_update()
	if not loading then
		if showerror then
			if btnp(❎) then
				showerror = false
			end
		elseif #carts > 0 then
			if btnp(➡️) then
				cpos = (cpos + 1)%numcarts
				fancy_updatecarts()
			end
			if btnp(⬅️) then
				cpos = (cpos - 1)%numcarts
				fancy_updatecarts()
			end
			dcpos = (cpos*0.5 + dcpos*1.5) / 2
			
			if btnp(❎) then
				if load and carttoload then
					load(carttoload)
				end
			end
		end
	else
		
		
		local i = loadnumber
		local v = cartnames[i]
		if loadnumber <= loadtotal then
			local newcart = {}
			newcart.name = v
			newcart.id = i - 1
			newcart.sprite = newcart.id%4
			newcart.text = {"a","e"}
			
			if __getlualine then
				local l = __getlualine(v,0)
				if sub(l,1,2) == '--' then
					l = sub(l,3,#l)
					if sub(l,1,1) == ' ' then
						l = sub(l,2,#l)
					end
					newcart.text[2] = l
					newcart.text[1] = v
				else
					newcart.text[1] = v
					newcart.text[2] = ''
				end
			else
				newcart.text[1] = v
				newcart.text[2] = 'name'
			end
			printh(newcart.text[1])
			
			add(carts,newcart)
			
			loadnumber += 1
		else
			fancy_updatecarts()
			loading = false
		end
		
	end
	
end

function fancy_drawcart(v)
	local cartx = (v.id-dcpos+1)*44+4
	local carty = 60
	local textx = cartx +16
	local texty = -6
	carty+= sin((v.id-dcpos+1)/4)*10
	texty-= sin((v.id-dcpos+1)/4)*24
	
	for li = 1,2 do
		local drawline = true
		if li == 2 then
			if v.text[1] == v.text[2] then
				drawline = false
			end
		end
		
		if drawline then
			print(v.text[li],textx-(#v.text[li])*2,texty+ li*8,textcolor)
		end
		
	end
	
	--base
	rectfill(cartx - 4,
	carty-6,
	cartx+35,
	carty+42,
	5)
	line(cartx-4,carty+43,cartx+34,carty+43,5)
	line(cartx-4,carty+44,cartx+33,carty+44,5)
	
	--border
	line(cartx-4,carty-7,cartx+35,carty-7,0)
	line(cartx-5,carty-6,cartx-5,carty+44,0)
	line(cartx+36,carty-6,cartx+36,carty+42,0)
	line(cartx+34,carty+44,cartx+36,carty+42,0)
	line(cartx-4,carty+45,cartx+33,carty+45,0)
	
	--label
	rect(cartx-1,carty-1,cartx+32,carty+32,0)
	spr(v.sprite*4,
		cartx,
		carty,
	4,4)
	
	--bottom label
	rect(cartx-1,carty+35,cartx+32,carty+42,0)
	line(cartx+1,carty+37,cartx+#v.name,carty+37,7)
	line(cartx+1,carty+39,cartx+(#v.name*3)%20+2,carty+39,7)
	
	line(cartx+1,carty+41,cartx+10,carty+41,6)
	--markings
	line(cartx-1,carty-3,cartx+2,carty-3,7)
	pset(cartx+4,carty-4,7)
	line(cartx+20,carty-4,cartx+32,carty-4,6)

	
end

function fancydrawmain()
	rectfill(0,0,128,128,bgcolor)
	if not loading then
		if #carts > 0 then
			for i,v in ipairs(carts) do
				fancy_drawcart(v)
			end
			--rect(44,44,83,83,7)
			--print(carts[cpos+1].name,0,0,textcolor)
			rect(0,120,127,127,textcolor)
			
			local barx = dcpos/(numcarts-1)*128-3
			print('◆',barx,121,textcolor)
			print('◆',barx,122,textcolor)
		else
			print('no carts found!',34,50,textcolor)
			local loctext = "place p8 carts in " .. cartpath
			print(loctext,64-(#loctext)*2,60,textcolor)
		end
	else
		color(textcolor)
		print('loading carts',37,50)
		local lstr = loadnumber .. ' / ' .. loadtotal
		print(lstr,64 - (#lstr*2),60)
	end
end


function fancy_draw()
	cls()
	clip()
	pal()
	palt(0,false)
	fancydrawmain()
	
	
	
	if showerror then
		rect(9,9,118,118,7)
		pal(darkenpal)
		clip(10,10,108,108)
		fancydrawmain()
		pal()
		for i,v in ipairs(errortable) do
			print(v,12,i*8+4,7)
		end
		
		
	end
end


-->8
--list

function list_init()
	carts={}
	numcarts = 0
	
	usestring = true
	
	showerror = false
	bioserror = ''
	errortable = {'error! press ❎ to close.'}
	if __getbioserror then
		bioserror = __getbioserror()
	end
	
	if bioserror != '' then
		showerror = true
		while true do
			if #bioserror >= 26 then
				add(errortable,sub(bioserror,1,26))
				bioserror = sub(bioserror,27,-1)
			else
				add(errortable,bioserror)
				break
			end
		end
	end
	
	if __getsetting then
			bgcolor = __getsetting('p8_bgcolor')
			textcolor = __getsetting('p8_textcolor')
	else
		bgcolor=1
		textcolor = 7
	end
	
	if bgcolor == 1 or bgcolor == 0 then
		bgcolor = 13
	end
	
	
	cidx=0
	carttoload = ""
	t=0
	linebuffer=""
	
	
	runcmd=false
	
	cpos = 0
	dcpos = 0
	
	if __listcarts then
		cartnames = __listcarts()
		numcarts = #cartnames
	else
		--cartnames = {'cart1.p8','cart2.p8.png','thirdcart.p8','cart4.p8','5cart.p8','c6.p8','theseventhcart.p8.png'}
		cartnames = {}
		numcarts = #cartnames
		usestring = false
	end
	
	loading = true
	
	loadtotal = #cartnames
	loadnumber = 1
	

	


end

function list_updatecarts()
	for i,v in ipairs(carts) do
		if v.id == cpos and not v.visible then
			v.visible = true
			
			if __loadlabel then
				__loadlabel(v.name,false,0)
			end
		
		else
			v.visible = false
		end
		if v.id == cpos then
			carttoload = v.name
		end
	end
end

function list_update()
	if not loading then
		if showerror then
			if btnp(❎) then
				showerror = false
			end
		elseif #carts > 0 then
			if btnp(⬇️) then
				cpos = (cpos + 1)%numcarts
				list_updatecarts()
			end
			if btnp(⬆️) then
				cpos = (cpos - 1)%numcarts
				list_updatecarts()
			end
			if btnp(⬅️) then
				cpos = (cpos - 7)%numcarts
				list_updatecarts()
			end
			if btnp(➡️) then
				cpos = (cpos + 7)%numcarts
				list_updatecarts()
			end
			dcpos = (cpos*0.5 + dcpos*1.5) / 2
			
			if btnp(❎) then
				if load and carttoload then
					load(carttoload)
				end
			end
		end
	else
		
		
		local i = loadnumber
		local v = cartnames[i]
		if loadnumber <= loadtotal then
			local newcart = {}
			newcart.name = v
			newcart.id = i - 1
			newcart.sprite = newcart.id%4
			newcart.text = {"a","e"}
			newcart.visible = false
			
			if __getlualine then
				local l = __getlualine(v,0)
				if sub(l,1,2) == '--' then
					l = sub(l,3,#l)
					if sub(l,1,1) == ' ' then
						l = sub(l,2,#l)
					end
					newcart.text[2] = l
					newcart.text[1] = v
				else
					newcart.text[1] = v
					newcart.text[2] = ''
				end
			else
				newcart.text[1] = v
				newcart.text[2] = 'name'
			end
			
			add(carts,newcart)
			
			loadnumber += 1
		else
			list_updatecarts()
			loading = false
		end
		
	end
	
end



function listdrawbg()
	rectfill(0,0,128,128,bgcolor)
	spr(0,0,0,16,16)
	clip(10,72,108,56)
	pal(darkenpal)
	spr(0,0,0,16,16)
	pal()
end


function list_draw()
	cls()
	
	
	
	
	if not loading then
		clip()
		pal()
		palt(0,false)
		listdrawbg()
		if showerror then
			rect(9,9,118,118,7)
			pal(darkenpal)
			clip(10,10,108,108)
			spr(0,0,0,16,16)
			pal()
			for i,v in ipairs(errortable) do
				print(v,12,i*8+4,7)
			end
			
			
		else
			print(cartpath,12,74,7)
			line(12,81,116,81,6)
			
			
			if #carts > 0 then
				clip(12,82,106,46)
				for i,v in ipairs(carts) do
					local drawpos = (i - 1 - dcpos)*7+0.5
					if cpos + 1 == i then
						
						local hoffset = 16 + #v.text[1]*4
						
						rectfill(12,99 + drawpos,hoffset,105+drawpos,bgcolor)
						
						rectfill(hoffset,99+drawpos,hoffset + (#v.text[2])*4+6,105+drawpos,textcolor)
						
						print(v.text[2],hoffset+4,100+drawpos,bgcolor)
						
						rect(0,98+drawpos,128,106+drawpos,0)
						
						color(textcolor)
					else
						color(7)
					end
					print(v.text[1],13,100+drawpos)
				end
			else
				print('no carts found!',14,86,7)
			end
		end
	else
		rectfill(0,0,128,128,bgcolor)
		color(textcolor)
		print('loading carts',37,50)
		local lstr = loadnumber .. ' / ' .. loadtotal
		print(lstr,64 - (#lstr*2),60)
	end
end


-->8
--runner

inits = {classic_init,fancy_init,list_init}
function _init()
	if __getsetting then
		theme = __getsetting('menustyle')
	else
		theme = 2
	end
	theme = theme % #inits --don't load a nonexistant menu style
	
	inits[theme+1]()
end

updates = {classic_update,fancy_update,list_update}
function _update60()
	updates[theme+1]()
end

draws = {classic_draw,fancy_draw,list_draw}
function _draw()	
	draws[theme+1]()
end



__gfx__
0123456789abcdef3333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888888888888cccccccccccccccccccccccccccccccc
0001155d55ddd1dd3333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888888888888cccccccccccccccccccccccccccccccc
33333333333377773333333333333333ddddddddddddddd77ddddddddddddddd88888888888888777788888888888888cccccccccc77777777cccccccccccccc
33333333333773337333333333333333ddddddddddddddd77ddddddddddddddd88888888888887788778888888888888cccccccccc7ccccccc7ccccccccccccc
33333333333733333773333333333333ddddddddddddddd77ddddddddddddddd88888888888877888887888888888888cccccccccccccccccc7ccccccccccccc
33333333337333333373333333333333dddddddddddddd7d7ddddddddddddddd88888888888778888888788888888888cccccccccccccccccc7ccccccccccccc
33333333337333333337333333333333ddddddddddddd7dd7ddddddddddddddd88888888888788888888788888888888cccccccccccccccccc7ccccccccccccc
33333333773333333333733333333333ddddddddddddd7dd7ddddddddddddddd88888888888888888888788888888888cccccccccccccccccc7ccccccccccccc
33333333773333333333373333333333dddddddddddd7ddd7ddddddddddddddd88888888888888888888788888888888cccccccccccccccccc7ccccccccccccc
33333337773333333333337333333333ddddddddddd77ddd7ddddddddddddddd88888888888888888888788888888888cccccccccccccccccc7ccccccccccccc
33333373733333333333337333333333dddddddddddddddd7ddddddddddddddd88888888888888888888788888888888cccccccccccccccc77cccccccccccccc
33333373333333333333337333333333dddddddddddddddd7ddddddddddddddd88888888888888888888788888888888ccccccccccccccc7777ccccccccccccc
33333373333333333333333733333333dddddddddddddddd7ddddddddddddddd88888888888888888888788888888888ccccccccccccccc77c7ccccccccccccc
33333337733333333333333733333333dddddddddddddddd7ddddddddddddddd88888888888888888888788888888888ccccccccccccccc7cc7ccccccccccccc
33333333373333333333337333333333ddddddddddddddd77ddddddddddddddd88888888888888888888788888888888cccccccccccccccccc7ccccccccccccc
33333333337733333333773333333333ddddddddddd77777777ddddddddddddd88888888888888888887888888888888cccccccccccccccccc7ccccccccccccc
33333333333773333777333333333333dddddddddddddddddd7777dddddddddd88888888888888888887888888888888ccccccccccccc77cc7cccccccccccccc
33333333333337777733333333333333dddddddddddddddddddddddddddddddd88888888888888888887888888888888ccccccccccccccc777cccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888878888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888877777788888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888877777888888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888777888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888888777888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888778888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888888888888cccccccccccccccccccccccccccccccc
33333333333333333333333333333333dddddddddddddddddddddddddddddddd88888888888888888888888888888888cccccccccccccccccccccccccccccccc
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777770777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777770777777077777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777770777777077777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777770777777077777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777770777770077777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777770777770000777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777770000000777077777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777770777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777770777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777770777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777770777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777770777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777770777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777777777777777777777777777777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777077777077007077777000000007777007777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77000077077077077077000000000077077077077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77777077777077770077770077777077077077777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77000077077077077077000000000077077077077000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
77000077077077007077777000000077770077770000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

)#";