namespace {
std::string manager_size(std::uint64_t bytes){
    char out[64];const char* unit="B";double value=double(bytes);
    if(bytes>=1073741824ULL){value/=1073741824.;unit="GB";}
    else if(bytes>=1048576){value/=1048576.;unit="MB";}
    else if(bytes>=1024){value/=1024.;unit="KB";}
    if(bytes<1024)std::snprintf(out,sizeof(out),"%llu B",static_cast<unsigned long long>(bytes));
    else std::snprintf(out,sizeof(out),"%.1f %s",value,unit);
    return out;
}
std::vector<std::string> manager_wrap(const std::string& value,int width){
    std::vector<std::string> rows;std::string row;
    for(std::size_t i=0;i<value.size();){
        const unsigned char c=static_cast<unsigned char>(value[i]);
        const auto length=std::min<std::size_t>(c<128?1:c<224?2:c<240?3:4,value.size()-i);
        const auto cp=value.substr(i,length);i+=length;
        if(!row.empty()&&measure(row+cp,18)>width){rows.push_back(row);row.clear();}
        row+=cp;
    }
    if(!row.empty())rows.push_back(row);
    return rows;
}
void manager_border(DecodedImageRGBA& p,int x,int y,int w,int h,Color c){
    rect(p,x,y,w,1,c);rect(p,x,y+h-1,w,1,c);rect(p,x,y,1,h,c);rect(p,x+w-1,y,1,h,c);
}
void manager_checkbox(DecodedImageRGBA& p,int x,int y,bool marked,bool copied){
    if(copied){line(p,x,y+6,x+5,y+11,Green,2);line(p,x+5,y+11,x+15,y,Green,2);}
    else{manager_border(p,x,y,14,14,marked?Blue:Muted);if(marked){line(p,x+3,y+6,x+6,y+10,Blue,2);line(p,x+6,y+10,x+12,y+2,Blue,2);}}
}
}
DecodedImageRGBA file_manager_hud(const FileManagerFix40::View& v){
    using Mode=FileManagerFix40::Mode;
    auto p=bitmap(Width,HudHeight);
    text(p,ProjectIdentity::Name,64,12,28,White,760);text(p,ProjectIdentity::Version,390,20,18,Muted,100);
    text(p,"Arquivos · HD / USB",960,19,20,Muted,255);text(p,v.status,64,59,20,White,1152);
    const bool has_selection=v.marked_count>0;
    for(int panel=0;panel<2;++panel){
        const int x=62+panel*585;const auto& pane=v.panels[panel];const bool active=v.active_panel==panel;
        rect(p,x,99,569,281,{{18,20,23,242}});
        manager_border(p,x,99,569,281,active?Blue:Color{{66,72,78,255}});
        const std::string side=panel==0?"esquerdo":"direito";
        text(p,has_selection?(v.selection_panel==panel?"Origem · ":"Destino · ")+side:"Painel "+side,x+18,111,20,active?White:Muted,520);
        text(p,pane.path.empty()?"Dispositivos":pane.path,x+18,138,18,Muted,530);
        const int count=int(pane.entries.size()),begin=std::max(0,std::min(pane.selected-3,count-7));
        for(int i=begin;i<std::min(count,begin+7);++i){
            const int y=167+(i-begin)*27;const auto& e=pane.entries[std::size_t(i)];
            if(e.marked)rect(p,x+12,y-2,545,26,{{87,130,177,85}});
            else if(e.copied)rect(p,x+12,y-2,545,26,{{53,116,82,75}});
            if(i==pane.selected)manager_border(p,x+12,y-2,545,26,active?White:Muted);
            manager_checkbox(p,x+23,y+4,e.marked,e.copied&&!e.marked);
            text(p,(e.directory?"Pasta · ":"")+e.name,x+48,y,20,e.marked?Blue:e.copied?Green:White,385);
            text(p,e.copied&&!e.marked?"Copiado":e.directory?"":manager_size(e.size),x+443,y+2,18,e.copied&&!e.marked?Green:Muted,106);
        }
        text(p,count?std::to_string(pane.selected+1)+" / "+std::to_string(count):"Pasta vazia",x+18,360,18,Muted,100);
        text(p,"L2/R2 · Página",x+390,360,18,Muted,158);
    }
    const auto& pane=v.panels[v.active_panel];
    const auto focused=pane.path.empty()||pane.entries.empty()?pane.path:pane.path+"/"+pane.entries[std::size_t(pane.selected)].name;
    text(p,has_selection?std::to_string(v.marked_count)+" itens marcados · Origem no painel "+(v.selection_panel==0?"esquerdo":"direito"):"Item em foco · SELECT mostra o caminho completo",64,392,18,has_selection?Blue:Muted,1152);
    text(p,focused,64,415,18,White,1152);
    if(v.mode==Mode::Browse){
        button(p,"X",73,456,"Abrir",Blue);button(p,"SQUARE",275,456,"Marcar",Blue);
        button(p,"L1/R1",491,456,"Painel");button(p,"TRIANGLE",760,456,"Ações",Green);button(p,"R3",1015,456,"Dispositivo");
        button(p,"O",73,491,"Voltar",Red);button(p,"SELECT",330,491,"Caminho");button(p,"START",1030,491,"Sair");
        return p;
    }
    rect(p,135,99,1010,340,{{18,20,23,255}});manager_border(p,135,99,1010,340,Color{{66,72,78,255}});
    auto heading=[&](const std::string& value){text(p,value,160,118,28,White,960);};
    auto row=[&](const std::string& value,int y,Color color=White){text(p,value,160,y,20,color,960);};
    auto path=[&](const std::string& value,int y){text(p,value,160,y,18,Muted,960);};
    auto option=[&](const std::string& label,int index,int selected,int y){
        if(index==selected){rect(p,149,y-3,982,29,{{106,141,152,100}});manager_border(p,149,y-3,982,29,Blue);}
        text(p,label,166,y,20,index==selected?White:Muted,955);
    };
    auto confirm=[&](const std::string& label){button(p,"X",73,471,label,Blue);button(p,"O",695,471,"Voltar",Red);};
    if(v.mode==Mode::Notice){
        heading("Gerenciador de arquivos");
        row("O fundo animado ficará pausado durante este modo.",180);
        row("Ao sair, sua configuração anterior será restaurada.",217,Muted);
        row("Marque a origem e abra o destino no outro painel.",275);
        row("Triângulo abre as ações; a cópia pede confirmação.",310);
        confirm("Continuar");
    }else if(v.mode==Mode::Actions){
        heading("Ações");
        const int from=has_selection?v.selection_panel:v.active_panel;
        const auto amount=has_selection?std::to_string(v.marked_count)+" itens":"item em foco";
        const std::string options[]={"Copiar "+amount+" para o painel "+(from==0?"direito":"esquerdo"),"Marcar todos","Limpar seleção","Atualizar pastas e dispositivos"};
        for(int i=0;i<4;++i)option(options[i],i,v.action_selected,181+i*37);
        path("Destino: "+v.panels[1-from].path,370);confirm("Escolher");
    }else if(v.mode==Mode::Devices){
        heading("Dispositivo do painel "+std::string(v.active_panel==0?"esquerdo":"direito"));
        const int count=int(v.devices.size()),begin=std::max(0,std::min(v.device_selected-3,count-7));
        if(!count)row("Nenhum dispositivo disponível.",184);
        for(int i=begin;i<std::min(count,begin+7);++i)option(v.devices[std::size_t(i)].label+" · "+v.devices[std::size_t(i)].path,i,v.device_selected,177+(i-begin)*32);
        confirm("Abrir dispositivo");
    }else if(v.mode==Mode::Details){
        heading("Caminho completo");
        const auto lines=manager_wrap(focused,960);const int pages=std::max(1,int((lines.size()+8)/9));
        const int page=std::min(v.details_page,pages-1);
        for(int i=page*9;i<std::min(int(lines.size()),(page+1)*9);++i)path(lines[std::size_t(i)],174+(i-page*9)*25);
        row("Página "+std::to_string(page+1)+" / "+std::to_string(pages),410,Muted);
        button(p,"L2/R2",88,471,"Página");button(p,"O",695,471,"Voltar",Red);
    }else if(v.mode==Mode::Planning||v.mode==Mode::Confirm){
        heading(v.mode==Mode::Planning?"Preparando a cópia":"Confirmar cópia");
        row(std::to_string(v.file_count)+" arquivos · "+std::to_string(v.folder_count)+" pastas",179);
        row("Tamanho total: "+manager_size(v.batch_total),214);
        row("Origem · painel "+std::string(v.source_panel==0?"esquerdo":"direito"),264,Blue);path(v.source_directory,292);
        row("Destino · painel "+std::string(v.source_panel==0?"direito":"esquerdo"),331,Green);path(v.destination_directory,359);
        if(v.mode==Mode::Planning){path(v.source,405);button(p,"O",73,471,"Cancelar preparação",Red);}
        else{row("Os arquivos existentes serão confirmados separadamente.",405,Muted);confirm("Iniciar cópia");}
    }else if(v.mode==Mode::Overwrite){
        heading("Arquivo existente no destino");
        path(v.destination,173);
        const char* options[]={"Sobrescrever este arquivo","Pular este arquivo","Sobrescrever todos","Pular todos","Cancelar o lote"};
        for(int i=0;i<5;++i)option(options[i],i,v.overwrite_selected,212+i*37);
        confirm("Confirmar");
    }else if(v.mode==Mode::Copy){
        heading("Copiando "+std::to_string(v.file_index)+" / "+std::to_string(v.file_count));
        path("De: "+v.source,175);path("Para: "+v.destination,203);
        const double file=v.total?std::clamp(double(v.done)/double(v.total),0.,1.):1.;
        const auto current=v.file_index>v.copied_count+v.skipped_count?v.done:0;
        const double all=v.batch_total?std::clamp(double(v.batch_done+current)/double(v.batch_total),0.,1.):1.;
        row("Arquivo atual · "+std::to_string(int(file*100))+"% · "+manager_size(v.done)+" / "+manager_size(v.total),247);
        rect(p,160,282,960,9,{{80,87,94,210}});rect(p,160,282,int(file*960),9,Blue);
        row("Lote inteiro · "+std::to_string(int(all*100))+"% · "+manager_size(v.batch_total),310);
        rect(p,160,345,960,12,{{80,87,94,210}});rect(p,160,345,int(all*960),12,Green);
        row(std::to_string(v.copied_count)+" copiados · "+std::to_string(v.skipped_count)+" ignorados · "+std::to_string(v.error_count)+" erros",389,Muted);
        button(p,"O",73,471,"Cancelar cópia",Red);button(p,"START",695,471,"Sair");
    }else if(v.mode==Mode::Summary){
        heading(v.error_count?"Operação interrompida":v.cancelled?"Operação cancelada":"Cópia concluída");
        row(std::to_string(v.copied_count)+" arquivos copiados",177,Green);
        row(std::to_string(v.skipped_count)+" arquivos ignorados · "+std::to_string(v.error_count)+" erros",218,v.error_count?Red:White);
        row("Copiado: "+manager_size(v.bytes_copied)+" · "+std::to_string(v.directories_done)+" pastas preparadas",260);
        const unsigned pending=v.file_count>=v.copied_count+v.skipped_count?v.file_count-v.copied_count-v.skipped_count:0;
        if(pending)row(std::to_string(pending)+" arquivos não concluídos",300,Muted);
        path("Destino: "+v.destination_directory,345);row("As cópias concluídas ficam destacadas na listagem.",391,Muted);
        confirm("Voltar à listagem");
    }else if(v.mode==Mode::ExitConfirm){
        heading("Cancelar a operação e sair?");
        row("Os arquivos concluídos serão preservados.",187);
        option("Continuar a operação",0,v.exit_selected,267);
        option("Cancelar a operação e sair",1,v.exit_selected,311);
        confirm("Confirmar");
    }
    return p;
}

